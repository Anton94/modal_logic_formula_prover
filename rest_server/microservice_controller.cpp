#include "microservice_controller.h"
#include "model.h"
#include "thread_termiator.h"

#include <iostream>
#include <string>

#include <filesystem/filesystem.hpp>

// using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

namespace
{
std::string to_string(bool b)
{
	return b ? "true" : "false";
}

bool starts_with(const std::string& s, const std::string& prefix)
{
	return s.find(prefix) == 0;
}

}

microservice_controller::microservice_controller(utility::string_t url)
	: m_listener(url)
	, looping_thread(&microservice_controller::remove_non_aciteve, this)
{
    m_listener.support(methods::GET,	
                       std::bind(&microservice_controller::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::POST,
                       std::bind(&microservice_controller::handle_post, this, std::placeholders::_1));
    m_listener.support(methods::PUT,
                       std::bind(&microservice_controller::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::DEL,
                       std::bind(&microservice_controller::handle_delete, this, std::placeholders::_1));

	srand(time(NULL));
}

microservice_controller::~microservice_controller()
{
	looping_thread.join();
}

void handle_error(pplx::task<void>& t)
{
    try
    {
        t.get();
    }
    catch(...)
    {
        // Ignore the error, Log it if a logger is available
    }
}

void microservice_controller::handle_get(http_request message)
{
    ucout << message.to_string() << std::endl;

    std::string message_path = utility::conversions::to_utf8string(message.absolute_uri().path());

    if(starts_with(message_path, "/rest"))
    {
        // for now we do not have any get get request
        message.reply(status_codes::OK);
        return;
    }

    fs::path relative_file(CLIENT_DIR + message_path);
    if(fs::exists(relative_file))
    {
        auto content_type = U("application/octet-stream");
        auto ext = relative_file.extension();
        if(ext.compare(".html") == 0)
        {
            content_type = U("text/html");
        }
        else if(ext.compare(".js") == 0)
        {
            content_type = U("text/javascript");
        }

        utility::string_t file_name_t = utility::conversions::to_string_t(relative_file);

        concurrency::streams::fstream::open_istream(file_name_t, std::ios::in)
            .then([=](concurrency::streams::istream is) {
                message.reply(status_codes::OK, is, content_type).then([](pplx::task<void> t) {
                    handle_error(t);
                });
            })
            .then([=](pplx::task<void> t) {
                try
                {
                    t.get();
                }
                catch(...)
                {
                    // opening the file (open_istream) failed.
                    // Reply with an error.
                    message.reply(status_codes::InternalError).then([](pplx::task<void> t) {
                        handle_error(t);
                    });
                }
            });

        return;
    }

    message.reply(status_codes::BadGateway);
}

void microservice_controller::handle_post(http_request message)
{
    ucout << message.to_string() << std::endl;

    std::string message_path = utility::conversions::to_utf8string(message.absolute_uri().path());

	if (starts_with(message_path, "/ping"))
	{
		message.content_ready().then([=](web::http::http_request request) {
			auto query_params = uri::split_query(request.request_uri().query());
			auto op_id_u = query_params.find(U("op_id"));
			if (op_id_u == query_params.end()) {
				message.reply(status_codes::BadRequest, "Missing 'op_id' query parameter.")
					.then([](pplx::task<void> t) { handle_error(t); });
				return;
			}

			auto op_id = utility::conversions::to_utf8string(op_id_u->second);
			std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_ctx_mutex_);

			if (op_id_to_cts_.find(op_id) == op_id_to_cts_.end())
			{
				message.reply(status_codes::BadRequest, "no such op_id.")
					.then([](pplx::task<void> t) { handle_error(t); });
			}
			else 
			{
				auto& res = op_id_to_task_result.find(op_id)->second;
				if (res.status_code == "FINISHED")
				{
					message.reply(status_codes::OK, res.to_string())
						.then([](pplx::task<void> t) { handle_error(t); });
					remove_op_id(op_id);
				}
				else
				{
					active_tasks.insert(op_id);
					std::string resulting_json = "{ 'status': '";
					resulting_json.append(res.status_code);
					resulting_json.append("' }");
					message.reply(status_codes::OK, resulting_json)
						.then([](pplx::task<void> t) { handle_error(t); });
				}
			}
		});
		return;
	}

	if (starts_with(message_path, "/cancel"))
	{
		message.content_ready().then([=](web::http::http_request request) {
			auto query_params = uri::split_query(request.request_uri().query());
			auto op_id_u = query_params.find(U("op_id"));
			if (op_id_u == query_params.end()) {
				message.reply(status_codes::BadRequest, "Missing 'op_id' query parameter.")
					.then([](pplx::task<void> t) { handle_error(t); });
				return;
			}

			auto op_id = utility::conversions::to_utf8string(op_id_u->second);

			std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_ctx_mutex_);
			auto it = op_id_to_cts_.find(op_id);
			bool was_removed = false;
			if (it != op_id_to_cts_.end() && op_id_to_task_result.find(op_id)->second.status_code != "FINISHED") {
				it->second.cancel();
				op_id_to_cts_.erase(it);
				active_tasks.insert(op_id);
				op_id_to_task_result.find(op_id)->second.status_code = "CANCELED";

				was_removed = true;
			}
				
			message.reply(status_codes::OK, was_removed ? "cancel true" : "cancel failed")
				.then([](pplx::task<void> t) { handle_error(t); });
		});
		return;
	}

	if (starts_with(message_path, "/task"))
	{
		// require lock, also check if the element is there if so generate new op_id.		
		
		struct id_token
		{
			std::string id;
			pplx::cancellation_token token;
		};
		auto generate_new_op_id_and_token = [&]() -> id_token
		{
			std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_ctx_mutex_);
			while (true)
			{
				std::string op_id = generate_random_op_id(10);
				if (op_id_to_cts_.find(op_id) != op_id_to_cts_.end())
				{
					continue;
				}
				active_tasks.insert(op_id);
				auto& task_res = op_id_to_task_result[op_id];
				task_res.status_code = "RUNNING";

				return { op_id, op_id_to_cts_[op_id].get_token() };
			}
		};

		auto id_token = generate_new_op_id_and_token();
		auto op_id = std::move(id_token.id);
		auto token = std::move(id_token.token);

		message.content_ready().then([=](web::http::http_request request) {
			auto query_params = uri::split_query(request.request_uri().query());
			auto algorithm_type_u = query_params.find(U("algorithm_type"));
			if (algorithm_type_u == query_params.end()) {
				message.reply(status_codes::BadRequest, "Missing 'algorithm_type' query parameter.")
					.then([](pplx::task<void> t) { handle_error(t); });
				return;
			}

			auto formula_filters_u = query_params.find(U("formula_filters"));
			if (formula_filters_u == query_params.end()) {
				message.reply(status_codes::BadRequest, "Missing 'formula_filters' query parameter.")
					.then([](pplx::task<void> t) { handle_error(t); });
				return;
			}
			message.reply(status_codes::OK, op_id).then([](pplx::task<void> t) { handle_error(t); });

			auto algorithm_type = utility::conversions::to_utf8string(algorithm_type_u->second);
			//auto formula = utility::conversions::to_utf8string(formula_u->second);
			auto formula_filters = utility::conversions::to_utf8string(formula_filters_u->second);

			request.extract_string(true)
				.then([=](string_t res) {
                const auto formula = utility::conversions::to_utf8string(web::uri().decode(res));

                set_termination_callback([&]() { return token.is_canceled(); }); // TODO: pass by value?

				try {
					formula_mgr mgr;
                    formula_mgr::formula_refiners formula_refs = extract_formula_refiners(formula_filters);

                    mgr.build(formula, formula_refs);
                    imodel* the_model;
					if (algorithm_type == "SLOW_MODEL")
					{
						the_model = new slow_model();
					}
					else if (algorithm_type == "FAST_MODEL")
					{
						the_model = new model();
					}
					else if (algorithm_type == "CONNECTIVITY")
					{
						the_model = new connected_model();
					}
					else if (algorithm_type == "BRUTEFORCE_MODEL")
					{
						the_model = new basic_bruteforce_model();
					}
					else {
						// assert since this should already be checked 
					}
					const auto is_satisfiable = mgr.is_satisfiable(*the_model);
					active_tasks.insert(op_id);
					op_id_to_task_result.find(op_id)->second.status_code = "FINISHED";
					op_id_to_task_result.find(op_id)->second.is_satisfied = is_satisfiable;
					op_id_to_task_result.find(op_id)->second.ids = the_model->get_variables_evaluations();
					op_id_to_task_result.find(op_id)->second.contacts = the_model->get_contact_relations();
					delete the_model;
				}
                catch (const TerminationException&) {
					info() << "Canceled ";
				}
				catch (...) {
					info() << "Canceled force";
				}
			}, token)
				.then([=](pplx::task<void> t) {
				try
				{
					try {
						t.wait();
					}
					catch (...) {
						info() << "Task failed due to cancelation";
					}
					
				}
				catch (...)
				{
					// opening the file (open_istream) failed.
					// Reply with an error.
					message.reply(status_codes::InternalError).then([](pplx::task<void> t) {
						handle_error(t);
					});
				}
			});
		});
		return;
	}

    if(starts_with(message_path, "/satisfy"))
    {
        message.content_ready().then([=](web::http::http_request request) {
            request.extract_string(true)
                .then([=](string_t res) {
                    const auto f_str = utility::conversions::to_utf8string(web::uri().decode(res));
                    //ucout << f_str << std::endl;
                    formula_mgr mgr;
                    mgr.build(f_str);

					basic_bruteforce_model bbm;
					bool isNativeSatisfied = mgr.brute_force_evaluate_with_points_count(bbm);

                    model m;
                    const auto is_satisfiable = mgr.is_satisfiable(m);

                    std::stringstream msg;
					msg << "Native? " << to_string(isNativeSatisfied) << "\n";
					for (model_points_set_t point : bbm.get_variables_evaluations())
					{
						msg << point << "\n";
					}
                    msg << "Satisfiable? " << to_string(is_satisfiable) << "\n";

                    const auto msg_t = utility::conversions::to_string_t(msg.str());
                    ucout << msg_t << std::endl;
                    message.reply(status_codes::OK, msg_t).then([](pplx::task<void> t) { handle_error(t); });
                })
                .then([=](pplx::task<void> t) {
                    try
                    {
                        t.get();
                    }
                    catch(...)
                    {
                        // opening the file (open_istream) failed.
                        // Reply with an error.
                        message.reply(status_codes::InternalError).then([](pplx::task<void> t) {
                            handle_error(t);
                        });
                    }
                });
        });
        return;
    }

    if(starts_with(message_path, "/build_formula"))
    {
        message.content_ready().then([=](web::http::http_request request) {
            request.extract_string(true)
                .then([=](string_t res) {
                    ucout << web::uri().decode(res) << std::endl;
                    const auto f_str = utility::conversions::to_utf8string(web::uri().decode(res));
                    formula_mgr mgr;
                    mgr.build(f_str);

                    std::stringstream built_formula;
                    built_formula << mgr;

                    string_t msg = utility::conversions::to_string_t(built_formula.str());

                    ucout << msg << std::endl;
                    // run the satisfier here
                    message.reply(status_codes::OK, msg).then([](pplx::task<void> t) { handle_error(t); });
                })
                .then([=](pplx::task<void> t) {
                    try
                    {
                        t.get();
                    }
                    catch(...)
                    {
                        // opening the file (open_istream) failed.
                        // Reply with an error.
                        message.reply(status_codes::InternalError).then([](pplx::task<void> t) {
                            handle_error(t);
                        });
                    }
                });
        });
        return;
    }

    if(starts_with(message_path, "/bruteforce_satisfy"))
    {
        message.content_ready().then([=](web::http::http_request request) {
            request.extract_string(true)
                .then([=](string_t res) {
                    ucout << web::uri().decode(res) << std::endl;
                    const auto f_str = utility::conversions::to_utf8string(web::uri().decode(res));
                    formula_mgr mgr;
                    mgr.build(f_str);

                    std::stringstream built_formula;
                    built_formula << mgr;
                    // true false -> mgr.brute_force_evaluate();
                    string_t msg = utility::conversions::to_string_t(built_formula.str());

                    ucout << msg << std::endl;
                    // run the satisfier here
                    message.reply(status_codes::OK, msg).then([](pplx::task<void> t) { handle_error(t); });
                })
                .then([=](pplx::task<void> t) {
                    try
                    {
                        t.get();
                    }
                    catch(...)
                    {
                        // opening the file (open_istream) failed.
                        // Reply with an error.
                        message.reply(status_codes::InternalError).then([](pplx::task<void> t) {
                            handle_error(t);
                        });
                    }
                });
        });
        return;
    }

    if(starts_with(message_path, "/is_satisfied_certificate"))
    {
        auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));
        auto queries = http::uri::split_query(message.relative_uri().query());
        auto run_bruteforce = queries.find(U("bruteforce"));
        bool bruteforce_enabled = run_bruteforce != queries.end() && (run_bruteforce->second == U("true"));

        message.content_ready().then([=](web::http::http_request request) {
            request.extract_string(true)
                .then([=](string_t res) {
                    const auto f_str = utility::conversions::to_utf8string(web::uri().decode(res));
                    //ucout << f_str << std::endl;

                    formula_mgr mgr;
                    mgr.build(f_str);

                    std::stringstream msg;
                    model m;
                    const auto is_satisfiable = mgr.is_satisfiable(m);
                    msg << "is_satisfiable: " << to_string(is_satisfiable) << "\n";

                    auto valid = true;

                    if(is_satisfiable)
                    {
                        /*msg << "cleaver method evaluations: " << out_evaluations << "\n";

                        const auto true_certificate = mgr.does_evaluates_to_true(out_evaluations);
                        msg << "cert valid: " << to_string(true_certificate) << "\n";

                        valid &= true_certificate;*/
                    }

                    if(bruteforce_enabled)
                    {
						basic_bruteforce_model bbm;
                        const auto bruteforce_status = mgr.brute_force_evaluate_with_points_count(bbm);
                        msg << "brute force: " << to_string(bruteforce_status) << "\n";
						if (bruteforce_status)
						{
							for (model_points_set_t point : bbm.get_variables_evaluations())
							{
								msg << point << "\n";
							}
						}

                        valid &= is_satisfiable == bruteforce_status;
                    }

                    msg << "Satisfiable? " << to_string(valid) << "\n";
                    const auto msg_t = utility::conversions::to_string_t(msg.str());
                    message.reply(status_codes::OK, msg_t).then([](pplx::task<void> t) { handle_error(t); });
                })
                .then([=](pplx::task<void> t) {
                    try
                    {
                        t.get();
                    }
                    catch(...)
                    {
                        // opening the file (open_istream) failed.
                        // Reply with an error.
                        message.reply(status_codes::InternalError).then([](pplx::task<void> t) {
                            handle_error(t);
                        });
                    }
                });
        });
        return;
    }

    message.reply(status_codes::BadGateway);
}

void microservice_controller::handle_put(http_request message)
{
    ucout << message.to_string() << std::endl;
    message.reply(status_codes::OK);
}

void microservice_controller::handle_delete(http_request message)
{
    ucout << message.to_string() << std::endl;
    message.reply(status_codes::OK);
}

std::string microservice_controller::generate_random_op_id(size_t length)
{
	auto randchar = []() -> char
	{
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}

void microservice_controller::remove_op_id(std::string op_id)
{
	op_id_to_cts_.erase(op_id);
	op_id_to_task_result.erase(op_id);
	active_tasks.erase(op_id);
}

auto microservice_controller::extract_formula_refiners(std::string formula_filters) -> formula_mgr::formula_refiners
{
	formula_mgr::formula_refiners formula_refs = formula_mgr::formula_refiners::none;

	std::string s = formula_filters;
	size_t pos = 0;
	std::string token;
	std::vector<std::string> xxx;
	while ((pos = s.find(",")) != std::string::npos) {
		token = s.substr(0, pos);
		xxx.push_back(token);
		s.erase(0, pos + 1);
	}

	for (int i = 0; i < xxx.size(); ++i)
	{
		if (xxx[i] == "convert_contact_less_eq_with_same_terms")
		{
			formula_refs |= formula_mgr::formula_refiners::convert_contact_less_eq_with_same_terms;
		}
		else if (xxx[i] == "convert_disjunction_in_contact_less_eq")
		{
			formula_refs |= formula_mgr::formula_refiners::convert_contact_less_eq_with_same_terms;
		}
		else if (xxx[i] == "reduce_constants")
		{
			formula_refs |= formula_mgr::formula_refiners::convert_contact_less_eq_with_same_terms;
		}
		else if (xxx[i] == "reduce_contacts_less_eq_with_constants")
		{
			formula_refs |= formula_mgr::formula_refiners::convert_contact_less_eq_with_same_terms;
		}
		else if (xxx[i] == "remove_double_negation")
		{
			formula_refs |= formula_mgr::formula_refiners::convert_contact_less_eq_with_same_terms;
		}
	}

	return formula_refs;
}
void microservice_controller::remove_non_aciteve()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(10));
		std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_ctx_mutex_);

		std::vector<std::string> known_op_ids;
        for (const auto& op_id_ctx : op_id_to_cts_)
		{
            const auto& op_id = op_id_ctx.first;
            known_op_ids.push_back(op_id);
		}
        for (const auto& known_op_id : known_op_ids)
		{
            if (active_tasks.find(known_op_id) == active_tasks.end())
			{
                op_id_to_cts_[known_op_id].cancel();
                op_id_to_cts_.erase(known_op_id);
                op_id_to_task_result.erase(known_op_id);
			}
		}
		active_tasks.clear();
	}
}
