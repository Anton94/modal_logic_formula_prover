﻿#include "microservice_controller.h"
#include "command_runner.h"
#include "model.h"
#include "thread_termiator.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

#include <filesystem/filesystem.hpp>

using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;
using namespace std::chrono_literals;

namespace
{
bool starts_with(const std::string& s, const std::string& prefix)
{
    return s.find(prefix) == 0;
}

static std::atomic_ulong running_tasks{};
}

microservice_controller::microservice_controller(const utility::string_t& url, size_t concurrent_tasks_limit,
                                                 const std::chrono::milliseconds& task_run_time_limit,
                                                 const size_t connected_model_max_used_variables)
    : m_listener(url)
    , concurrent_tasks_limit_(concurrent_tasks_limit)
    , task_run_time_limit_(task_run_time_limit)
    , connected_model_max_used_variables_(connected_model_max_used_variables)
{
    info() << "Creating a server with url " << utility::conversions::to_utf8string(url)
           << ", concurrent tasks limit " << concurrent_tasks_limit_ << ", task run time limit "
           << task_run_time_limit_.count() << "ms.";

    m_listener.support(methods::GET,
                       std::bind(&microservice_controller::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::POST,
                       std::bind(&microservice_controller::handle_post, this, std::placeholders::_1));
    m_listener.support(methods::PUT,
                       std::bind(&microservice_controller::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::DEL,
                       std::bind(&microservice_controller::handle_delete, this, std::placeholders::_1));

    srand(static_cast<unsigned>(time(nullptr)));
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

void microservice_controller::handle_get(const http_request& message)
{

    std::string message_path = utility::conversions::to_utf8string(message.absolute_uri().path());

    // TODO: parse this as URL and then get path, query strings and so on.
    if(starts_with(message_path, "/rest"))
    {
        if(starts_with(message_path, "/rest/ping"))
        {
            handle_ping(message);
            return;
        }
        ucout << message.to_string() << std::endl;

        if(starts_with(message_path, "/rest/cancel"))
        {
            handle_cancel(message);
            return;
        }

        message.reply(status_codes::OK);
        return;
    }
    ucout << message.to_string() << std::endl;

	// Eventual enchancment is to read the files and folders in the simple_client on boot and 
	// only allow these files to be requested afterwards.
	// Since now we do not have many files this is good to go.
	if (message_path != "/index.html"
		&& message_path != "/home.html"
		&& message_path != "/howto.html"
        && message_path != "/samples.html"
        && message_path != "/howto.html"
        && message_path != "/api_wrapper.js"
		&& message_path != "/server_api.js"
		&& message_path != "/styles.css"
		&& message_path != "/visualizer.js"
		&& message_path != "/lib/jquery.min.js"
		&& message_path != "/lib/d3.v4.js"
		&& message_path != "/img/home-icon.png") {
		message.reply(status_codes::BadGateway);
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
		else if (ext.compare(".css") == 0)
		{
			content_type = U("text/css");
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

void microservice_controller::handle_post(const http_request& message)
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

void microservice_controller::handle_put(const http_request& message)
{
    ucout << message.to_string() << std::endl;
    message.reply(status_codes::OK);
}

void microservice_controller::handle_delete(const http_request& message)
{
    ucout << message.to_string() << std::endl;
    message.reply(status_codes::OK);
}

void microservice_controller::handle_task(const http_request& message)
{
    if(running_tasks >= concurrent_tasks_limit_)
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
        std::string op_id = generate_random_op_id(10);
        while(op_id_to_task_info_.find(op_id) != op_id_to_task_info_.end())
        {
            op_id = generate_random_op_id(10);
        }

        auto& task_info = op_id_to_task_info_[op_id];
        task_info.start =
            std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now())
                .time_since_epoch();
        return {op_id, task_info.get_token()};
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

        const auto& algorithm_type = utility::conversions::to_utf8string(algorithm_type_u->second);
        const auto& formula_filters = utility::conversions::to_utf8string(formula_filters_u->second);

        request.extract_string(true)
            .then(
                [=](string_t res)
                {
                    ++running_tasks;

                    const auto formula = utility::conversions::to_utf8string(web::uri().decode(res));

                    bool is_task_info_removed = false;
                    std::stringstream info_output;

                    const auto UPDATE_TIME = 1s;
                    std::chrono::steady_clock::time_point next_update =
                        std::chrono::steady_clock::now() + UPDATE_TIME;
                    auto stream_accumulated_output = [&]()
                    {
                        const auto now = std::chrono::steady_clock::now();
                        if(next_update < now)
                        {
                            std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);
                            auto task_info_it = op_id_to_task_info_.find(op_id);
                            if(task_info_it != op_id_to_task_info_.end())
                            {
                                auto& final_result = task_info_it->second.result_;
                                final_result.output.append(info_output.str());

                                info_output.str(std::string());
                                next_update = now + UPDATE_TIME;
                            }
                            else if(!is_task_info_removed)
                            {
                                std::cout << "The task info for: " << op_id << " has been removed, so will terminate the algorithm's execution." << std::endl;
                                is_task_info_removed = true;

                                info_output.clear();
                                next_update = now + 1s;
                            }
                        }
                    };

                    set_termination_callback([&]() { // a little ugly but the accumulated output should be
                                                     // updated more often, not only when printing something,
                        // because there might be huge intervals in which the algorithm does not print
                        // anything and some of the already accumulated log info will not be pushed to the
                        // final_result's output
                        // till a new log message is received.
                        // We want to provide all the accumulated output to that point so when there is no new
                        // output the user will know where the algorithm is slow.
                        stream_accumulated_output();
                        return token.is_canceled() || is_task_info_removed;
                    }); // TODO: pass by value?

                    // set_verbose_logger([&](std::stringstream&& s) { info_output << "Verbose: " <<
                    // s.rdbuf() << "\n"; stream_accumulated_output();  });
                    // if the trace on user preference.
                    set_trace_logger([&](std::stringstream&& s) {
                        info_output << "Trace: " << s.rdbuf() << "\n";
                        stream_accumulated_output();
                    });
                    set_info_logger([&](std::stringstream&& s) {
                        info_output << "Info: " << s.rdbuf() << "\n";
                        stream_accumulated_output();
                    });
                    set_error_logger([&](std::stringstream&& s) {
                        info_output << "Error: " << s.rdbuf() << "\n";
                        stream_accumulated_output();
                    });

                    bool is_parsed{};
                    bool is_satisfiable{};
                    try
                    {
                        formula_mgr mgr;
                        formula_mgr::formula_refiners formula_refs =
                            extract_formula_refiners(formula_filters);

                        is_parsed = mgr.build(formula, formula_refs);

                        std::unique_ptr<imodel> the_model;
                        if(algorithm_type == "MEASURED_MODEL")
                        {
                            the_model = std::make_unique<measured_model>();
                        }
                        else if(algorithm_type == "FAST_MODEL")
                        {
                            the_model = std::make_unique<model>();
                        }
                        else if(algorithm_type == "CONNECTIVITY")
                        {
                            the_model = std::make_unique<connected_model>(connected_model_max_used_variables_);
                        }
                        else if(algorithm_type == "BRUTEFORCE_MODEL")
                        {
                            the_model = std::make_unique<basic_bruteforce_model>();
                        }
                        else
                        {
                            error() << "Something went wrong, received unrecognized model type from JS. "
                                       "Fallback to FAST MODEL";
                            the_model = std::make_unique<model>();
                        }

                        if(is_parsed)
                        {
                            is_satisfiable = mgr.is_satisfiable(*the_model);
                        }

                        // check this find here for not present
                        std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);
                        auto task_info_it = op_id_to_task_info_.find(op_id);
                        if(task_info_it != op_id_to_task_info_.end())
                        {
                            auto& final_result = task_info_it->second.result_;
                            final_result.status_code = "FINISHED";
                            final_result.is_parsed = is_parsed;
                            final_result.is_satisfied = is_satisfiable;
                            final_result.output.append(info_output.str());

                            if(is_parsed && is_satisfiable)
                            {
                                final_result.variables = mgr.get_variables();
                                final_result.ids = the_model->get_variables_evaluations();
                                final_result.contacts = the_model->get_contact_relations();
                            }
                        }
                        else
                        {
                            std::cout << "The task info for: " << op_id << " has been removed, so will not update it's result." << std::endl;
                        }
                    }
                    catch(const TerminationException&)
                    {
                        info() << "The task was canceled.";

                        std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);
                        auto task_info_it = op_id_to_task_info_.find(op_id);
                        if(task_info_it != op_id_to_task_info_.end())
                        {
                            auto& final_result = task_info_it->second.result_;
                            final_result.status_code = "CANCELED";
                            final_result.output.append(info_output.str());
                        }
                        else
                        {
                            std::cout << "The task info for: " << op_id << " has been removed, so will not update it's result." << std::endl;
                        }
                    }
                    catch(...)
                    {
                        // TODO: add some general error??
                    }
                    --running_tasks;
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
                        info() << "Task failed due to cancelation"; // TODO: this info might be not
                                                                    // initialized and therefre a NOOP if this
                                                                    // is ran in a separate task
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

void microservice_controller::handle_cancel(const http_request& message)
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

        const auto& op_id = utility::conversions::to_utf8string(op_id_u->second);

        std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);
        auto it = op_id_to_task_info_.find(op_id);
        bool was_removed = false;
        if(it != op_id_to_task_info_.end() && it->second.result_.status_code != "FINISHED")
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

void microservice_controller::handle_ping(const http_request& message)
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

        const auto& op_id = utility::conversions::to_utf8string(op_id_u->second);
        std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);

        if(op_id_to_task_info_.find(op_id) == op_id_to_task_info_.end())
        {
            message.reply(status_codes::BadRequest, "no such op_id.").then([](pplx::task<void> t) {
                handle_error(t);
            });
        }
        else
        {
            std::cout << "Received ping from " << op_id << std::endl;
            auto task_info_it = op_id_to_task_info_.find(op_id);
            if(task_info_it != op_id_to_task_info_.end())
            {
                auto& info = task_info_it->second;
                auto& res = info.result_;
                info.activate();

                const auto now_ms =
                    std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now())
                        .time_since_epoch();
                const auto ellapsed_time = now_ms - info.start;
                if(ellapsed_time > task_run_time_limit_)
                {
                    std::cout << "Task with op_id:" << op_id << " is taking more than "
                           << task_run_time_limit_.count() << "ms, so it will be terminated." << std::endl;
                    res.output +=
                        "The task is taking more than " + std::to_string(task_run_time_limit_.count()) +
                        "ms and will be terminated. Try to simplify the formula or use a faster model "
                        "algorithm(if available).";
                    res.status_code = "CANCELED";
                }

                message.reply(status_codes::OK, res.to_string()).then([](pplx::task<void> t) {
                    handle_error(t);
                });

                res.output.clear();

                if(res.status_code != "RUNNING")
                {
                    remove_op_id(op_id);
                }
            }
        }
    });
}

void microservice_controller::handle_formula_generation(const http_request& message)
{
    message.content_ready().then([=](web::http::http_request request) {
        auto query_params = uri::split_query(request.request_uri().query());

        auto get_param = [&](const std::string& param, std::string& param_value) {
            auto op_id_u = query_params.find(utility::conversions::to_string_t(param));
            if(op_id_u == query_params.end())
            {
                message.reply(status_codes::BadRequest, "Missing 'op_id' query parameter.")
                    .then([](pplx::task<void> t) { handle_error(t); });
                return false;
            }

            param_value = utility::conversions::to_utf8string(op_id_u->second);
            return true;
        };

        std::string max_term_length;
        std::string min_variables_count;
        std::string max_variables_count;
        std::string max_existence_rules;
        std::string min_non_existence_rules;
        std::string max_non_existence_rules;
        std::string formulas;

        if(!get_param("max_term_length", max_term_length) ||
           !get_param("min_variables_count", min_variables_count) ||
           !get_param("max_variables_count", max_variables_count) ||
           !get_param("max_existence_rules", max_existence_rules) ||
           !get_param("min_non_existence_rules", min_non_existence_rules) ||
           !get_param("max_non_existence_rules", max_non_existence_rules) || !get_param("formulas", formulas))
        {
            return;
        }

        // TODO: some guards for the arugments, e.g. max number of formulas, max term length, etc. in order to
        // restrict the running time of the script to some reasonable value.

        static std::atomic<unsigned> output_file_id{};
        std::string output_file =
            "formula_generator_outputs/out_" + std::to_string(output_file_id++) + ".txt";
        std::string output;
        std::string cmd = "python3 ../../tools/formula_generator.py"; // TODO: proper path to the script,
                                                                      // maybe copy the script to the build
                                                                      // folder, etc
        cmd += " --output_filename " + output_file;
        cmd += " --max_term_length " + max_term_length;
        cmd += " --min_variables_count " + min_variables_count;
        cmd += " --max_variables_count " + max_variables_count;
        cmd += " --max_existence_rules " + max_existence_rules;
        cmd += " --min_non_existence_rules " + min_non_existence_rules;
        cmd += " --max_non_existence_rules " + max_non_existence_rules;
        cmd += " --formulas " + formulas;

        if(!command_runner::run(cmd, output))
        {
            error() << "Something went wrong with running the script: " << cmd << "\n"
                    << "Output:\n"
                    << output;
            message.reply(status_codes::OK, "Sorry, unable to generate formulas, try changing the values.")
                .then([](pplx::task<void> t) { handle_error(t); });
            return;
        }

        std::ifstream in(output_file);
        std::string generated_formulas;

        in.seekg(0, std::ios::end);
        const auto size = in.tellg();
        if(size < 0)
        {
            error() << "Something went wrong with the produced output file from formula_generator script: "
                    << cmd << "\n"
                    << "Output file: " << output_file;
            message.reply(status_codes::OK, "Sorry, unable to generate formulas. Contact an administrator.")
                .then([](pplx::task<void> t) { handle_error(t); });
            return;
        }

        generated_formulas.reserve(static_cast<size_t>(size));
        in.seekg(0, std::ios::beg);

        generated_formulas.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        message.reply(status_codes::OK, generated_formulas).then([](pplx::task<void> t) { handle_error(t); });

        if(remove(output_file.c_str()))
        {
            error() << "Unable to remove a generated output file: " << output_file;
        }
    });
}

std::string microservice_controller::generate_random_op_id(size_t length)
{
    auto randchar = []() -> char {
        const char charset[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = sizeof(charset) - 1;
        return charset[static_cast<unsigned>(rand()) % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

void microservice_controller::remove_op_id(const std::string& op_id)
{
    op_id_to_task_info_.erase(op_id);
}

auto microservice_controller::extract_formula_refiners(const std::string& formula_filters)
    -> formula_mgr::formula_refiners
{
    formula_mgr::formula_refiners formula_refs = formula_mgr::formula_refiners::none;

    std::string s = formula_filters;
    size_t pos = 0;
    std::string token;
    std::vector<std::string> filters;
    while((pos = s.find(',')) != std::string::npos)
    {
        token = s.substr(0, pos);
        filters.push_back(token);
        s.erase(0, pos + 1);
    }

    for(const auto& filter : filters)
    {
        if(filter == "convert_contact_less_eq_with_same_terms")
        {
            formula_refs |= formula_mgr::formula_refiners::convert_contact_less_eq_with_same_terms;
        }
        else if(filter == "convert_disjunction_in_contact_less_eq")
        {
            formula_refs |= formula_mgr::formula_refiners::convert_disjunction_in_contact_less_eq;
        }
        else if(filter == "reduce_constants")
        {
            formula_refs |= formula_mgr::formula_refiners::reduce_constants;
        }
        else if(filter == "reduce_contacts_less_eq_with_constants")
        {
            formula_refs |= formula_mgr::formula_refiners::reduce_contacts_with_constants;
        }
        else if(filter == "remove_double_negation")
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
        if(!op_id_ctx.second.is_active())
        {
            op_ids_to_remove.push_back(op_id_ctx.first);
        }
        op_id_ctx.second.deactivate();
    }

    for(const auto& op_id : op_ids_to_remove)
    {
        std::cout << "Removing non-active task with id: " << op_id << std::endl;
        op_id_to_task_info_.erase(op_id);
    }
}

void microservice_controller::print_info()
{
    std::lock_guard<std::mutex> op_id_to_ctx_guard(op_id_to_task_info_mutex_);
    std::cout << "Running " << running_tasks << " concurrent tasks with task infos for " << op_id_to_task_info_.size() << " tasks." << std::endl;
}
