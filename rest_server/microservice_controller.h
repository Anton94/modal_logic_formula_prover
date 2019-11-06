#pragma once

#include "library.h"
#include "task_result.h"
#include "task_info.h"

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

    pplx::task<void> open()
    {
        return m_listener.open();
    }
    pplx::task<void> close()
    {
        return m_listener.close();
    }

    void remove_non_active();

private:
    void handle_get(http_request message);
    void handle_post(http_request message);
    void handle_put(http_request message);
    void handle_delete(http_request message);

    http_listener m_listener;

    std::string generate_random_op_id(size_t length);
    void remove_op_id(std::string op_id);

    auto extract_formula_refiners(std::string formula_filters) -> formula_mgr::formula_refiners;

    std::mutex op_id_to_task_info_mutex_;
    std::unordered_map<std::string, task_info> op_id_to_task_info_;

    std::string CLIENT_DIR = "../client/dist";
};
