#pragma once

#include "library.h"
#include "task_info.h"
#include "task_result.h"

#include <cpprest/asyncrt_utils.h>
#include <cpprest/containerstream.h>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>
#include <thread>

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class microservice_controller
{
public:
    microservice_controller(const utility::string_t& url, size_t concurrent_tasks_limit,
                            const std::chrono::milliseconds& task_run_time_limit,
                            const size_t connected_model_max_used_variables);

    pplx::task<void> open()
    {
        return m_listener.open();
    }
    pplx::task<void> close()
    {
        return m_listener.close();
    }

    void remove_non_active();

    void print_info();

private:
    void handle_get(const http_request& message);
    void handle_post(const http_request& message);
    void handle_put(const http_request& message);
    void handle_delete(const http_request& message);

    void handle_task(const http_request& message);
    void handle_cancel(const http_request& message);
    void handle_ping(const http_request& message);
    void handle_formula_generation(const http_request& message);

    std::string generate_random_op_id(size_t length);
    void remove_op_id(const std::string& op_id);

    auto extract_formula_refiners(const std::string& formula_filters) -> formula_mgr::formula_refiners;

    bool is_requests_limit_exceeded();

    http_listener m_listener;

    std::mutex op_id_to_task_info_mutex_;
    std::unordered_map<std::string, task_info> op_id_to_task_info_;

    std::mutex requests_limit_mutex_;
    const size_t concurrent_tasks_limit_;
    const std::chrono::milliseconds task_run_time_limit_;
    const size_t connected_model_max_used_variables_;

    std::string CLIENT_DIR = "../client/dist";
};
