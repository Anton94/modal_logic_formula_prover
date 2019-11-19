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
	: microservice_controller(url, 1000)
{
}

microservice_controller::microservice_controller(utility::string_t url, size_t requests_limit)
	: m_listener(url)
	, requests_limit_(requests_limit)
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

    // TODO: parse this as URL and then get path, query strings and so on.
    if(starts_with(message_path, "/rest"))
    {
        if(starts_with(message_path, "/rest/ping"))
        {
            handle_ping(message);
            return;
        }

        if(starts_with(message_path, "/rest/cancel"))
        {
            handle_cancel(message);
            return;
        }

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

    if(starts_with(message_path, "/task"))
    {
        handle_task(message);
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

void microservice_controller::handle_task(http_request message)
{
	// TODO: do we need a mutex lock for this ? 
	if (op_id_to_task_info_.size() > requests_limit_)
	{
		message.reply(status_codes::TooManyRequests, "There are too many requests, try again later.")
			.then([](pplx::task<void> t) { handle_error(t); });
		return;
	}

    struct id_token
    {
        std::string id;
        pplx::cancellation_token token;
    };
    auto generate_new_op_id_and_token = [&]() -> id_token {
        std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);
        while(true)
        {
            std::string op_id = generate_random_op_id(10);
            if(op_id_to_task_info_.find(op_id) != op_id_to_task_info_.end())
            {
                continue;
            }

            return {op_id, op_id_to_task_info_[op_id].get_token()};
        }
    };

    auto id_token = generate_new_op_id_and_token();
    auto op_id = std::move(id_token.id);
    auto token = std::move(id_token.token);

    message.content_ready().then([=](web::http::http_request request) {
        auto query_params = uri::split_query(request.request_uri().query());
        auto algorithm_type_u = query_params.find(U("algorithm_type"));
        if(algorithm_type_u == query_params.end())
        {
            message.reply(status_codes::BadRequest, "Missing 'algorithm_type' query parameter.")
                .then([](pplx::task<void> t) { handle_error(t); });
            return;
        }

        auto formula_filters_u = query_params.find(U("formula_filters"));
        if(formula_filters_u == query_params.end())
        {
            message.reply(status_codes::BadRequest, "Missing 'formula_filters' query parameter.")
                .then([](pplx::task<void> t) { handle_error(t); });
            return;
        }
        message.reply(status_codes::OK, op_id).then([](pplx::task<void> t) { handle_error(t); });

        auto algorithm_type = utility::conversions::to_utf8string(algorithm_type_u->second);
        auto formula_filters = utility::conversions::to_utf8string(formula_filters_u->second);

        request.extract_string(true)
            .then(
                [=](string_t res) {
                    const auto formula = utility::conversions::to_utf8string(web::uri().decode(res));

                    set_termination_callback(
                        [&]() { return token.is_canceled(); }); // TODO: pass by value?

                    try
                    {
                        std::stringstream info_output;
                        //set_verbose_logger([](std::stringstream&& s) { std::cout << "Verbose: " << s.rdbuf() << std::endl; });
                        //set_trace_logger([](std::stringstream&& s) { std::cout << "Trace: " << s.rdbuf() << std::endl; });
                        set_info_logger([&](std::stringstream&& s) { info_output << "Info: " << s.rdbuf() << "\n"; });
                        //set_error_logger([](std::stringstream&& s) { std::cout << "Error: " << s.rdbuf() << std::endl; });
                        formula_mgr mgr;
                        formula_mgr::formula_refiners formula_refs =
                            extract_formula_refiners(formula_filters);

                        bool is_parsed = mgr.build(formula, formula_refs);
                        imodel* the_model;
                        if(algorithm_type == "MEASURED_MODEL")
                        {
                            the_model = new measured_model();
                        }
                        else if(algorithm_type == "FAST_MODEL")
                        {
                            the_model = new model();
                        }
                        else if(algorithm_type == "CONNECTIVITY")
                        {
                            the_model = new connected_model();
                        }
                        else if(algorithm_type == "BRUTEFORCE_MODEL")
                        {
                            the_model = new basic_bruteforce_model();
                        }
                        else
                        {
                            // assert since this should already be checked
                        }
                        const auto is_satisfiable = (is_parsed) ? mgr.is_satisfiable(*the_model) : false;

                        // check this find here for not present
                        std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);
                        auto& final_result = op_id_to_task_info_.find(op_id)->second.result_;
                        final_result.status_code = "FINISHED";
                        final_result.is_parsed = is_parsed;
                        final_result.is_satisfied = is_satisfiable;
                        final_result.output = info_output.str();

                        if (is_parsed && is_satisfiable)
                        {
                            final_result.ids = the_model->get_variables_evaluations();
                            final_result.contacts = the_model->get_contact_relations();
                        }

                        delete the_model;
                    }
                    catch(const TerminationException&)
                    {
                        info() << "Canceled ";
                    }
                    catch(...)
                    {
                        info() << "Canceled force";
                    }
                },
                token)
            .then([=](pplx::task<void> t) {
                try
                {
                    try
                    {
                        t.wait();
                    }
                    catch(...)
                    {
                        info() << "Task failed due to cancelation";
                    }
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
}

void microservice_controller::handle_cancel(http_request message)
{
    message.content_ready().then([=](web::http::http_request request) {
        auto query_params = uri::split_query(request.request_uri().query());
        auto op_id_u = query_params.find(U("op_id"));
        if(op_id_u == query_params.end())
        {
            message.reply(status_codes::BadRequest, "Missing 'op_id' query parameter.")
                .then([](pplx::task<void> t) { handle_error(t); });
            return;
        }

        auto op_id = utility::conversions::to_utf8string(op_id_u->second);

        std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);
        auto it = op_id_to_task_info_.find(op_id);
        bool was_removed = false;
        if(it != op_id_to_task_info_.end() &&
           it->second.result_.status_code != "FINISHED")
        {
            it->second.cancel();
            it->second.activate();
            it->second.result_.status_code = "CANCELED";

            was_removed = true;
        }

        message.reply(status_codes::OK, was_removed ? "cancel true" : "cancel failed")
            .then([](pplx::task<void> t) { handle_error(t); });
    });
}

void microservice_controller::handle_ping(http_request message)
{
    message.content_ready().then([=](web::http::http_request request) {
        auto query_params = uri::split_query(request.request_uri().query());
        auto op_id_u = query_params.find(U("op_id"));
        if(op_id_u == query_params.end())
        {
            message.reply(status_codes::BadRequest, "Missing 'op_id' query parameter.")
                .then([](pplx::task<void> t) { handle_error(t); });
            return;
        }

        auto op_id = utility::conversions::to_utf8string(op_id_u->second);
        std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);

        if(op_id_to_task_info_.find(op_id) == op_id_to_task_info_.end())
        {
            message.reply(status_codes::BadRequest, "no such op_id.").then([](pplx::task<void> t) {
                handle_error(t);
            });
        }
        else
        {
            auto& info = op_id_to_task_info_.find(op_id)->second;
            auto& res = info.result_;
            if(res.status_code == "FINISHED")
            {
                message.reply(status_codes::OK, res.to_string()).then([](pplx::task<void> t) {
                    handle_error(t);
                });
                remove_op_id(op_id);
            }
            else
            {
                info.activate();
                std::string resulting_json = "{ 'status': '";
                resulting_json.append(res.status_code);
                resulting_json.append("' }");
                message.reply(status_codes::OK, resulting_json).then([](pplx::task<void> t) {
                    handle_error(t);
                });
            }
        }
    });
}

std::string microservice_controller::generate_random_op_id(size_t length)
{
    auto randchar = []() -> char {
        const char charset[] = "0123456789"
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
    op_id_to_task_info_.erase(op_id);
}

auto microservice_controller::extract_formula_refiners(std::string formula_filters)
    -> formula_mgr::formula_refiners
{
    formula_mgr::formula_refiners formula_refs = formula_mgr::formula_refiners::none;

    std::string s = formula_filters;
    size_t pos = 0;
    std::string token;
    std::vector<std::string> xxx;
    while((pos = s.find(",")) != std::string::npos)
    {
        token = s.substr(0, pos);
        xxx.push_back(token);
        s.erase(0, pos + 1);
    }

    for(int i = 0; i < xxx.size(); ++i)
    {
        if(xxx[i] == "convert_contact_less_eq_with_same_terms")
        {
            formula_refs |= formula_mgr::formula_refiners::convert_contact_less_eq_with_same_terms;
        }
        else if(xxx[i] == "convert_disjunction_in_contact_less_eq")
        {
            formula_refs |= formula_mgr::formula_refiners::convert_disjunction_in_contact_less_eq;
        }
        else if(xxx[i] == "reduce_constants")
        {
            formula_refs |= formula_mgr::formula_refiners::reduce_constants;
        }
        else if(xxx[i] == "reduce_contacts_less_eq_with_constants")
        {
            formula_refs |= formula_mgr::formula_refiners::reduce_contacts_with_constants;
        }
        else if(xxx[i] == "remove_double_negation")
        {
            formula_refs |= formula_mgr::formula_refiners::remove_double_negation;
        }
    }

    return formula_refs;
}

void microservice_controller::remove_non_active()
{
    std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);

    std::vector<std::string> op_ids_to_remove;
    for(auto& op_id_ctx : op_id_to_task_info_)
    {
        if (!op_id_ctx.second.is_active())
        {
            op_ids_to_remove.push_back(op_id_ctx.first);
        }
        op_id_ctx.second.deactivate();
    }

    for(const auto& op_id : op_ids_to_remove)
    {
        op_id_to_task_info_.erase(op_id);
    }
}
