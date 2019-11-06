#pragma once

#include "library.h"
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
    microservice_controller(utility::string_t url);
    ~microservice_controller();

    pplx::task<void> open()
    {
        return m_listener.open();
    }
    pplx::task<void> close()
    {
        return m_listener.close();
    }

	void remove_non_aciteve();

private:
    void handle_get(http_request message);
    void handle_post(http_request message);
    void handle_put(http_request message);
    void handle_delete(http_request message);

    http_listener m_listener;

    std::string generate_random_op_id(size_t length);
    void remove_op_id(std::string op_id);

    pplx::cancellation_token_source cts_;

    auto extract_formula_refiners(std::string formula_filters) -> formula_mgr::formula_refiners;

    std::mutex op_id_to_ctx_mutex_;
    std::unordered_map<std::string, pplx::cancellation_token_source> op_id_to_cts_;
    std::unordered_set<std::string> active_tasks;
    std::unordered_map<std::string, task_result> op_id_to_task_result;
    std::thread looping_thread;
    std::string CLIENT_DIR = "../client/dist";
};
