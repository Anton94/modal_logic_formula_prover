#pragma once

#include "library.h"

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

private:
    void handle_get(http_request message);
    void handle_post(http_request message);
    void handle_put(http_request message);
    void handle_delete(http_request message);

    http_listener m_listener;

    std::string generate_random_op_id(size_t length);
    void remove_op_id(std::string op_id);

    pplx::cancellation_token_source cts_;

    struct task_result
    {
        std::string status_code; // enum "RUNNING", "CANCELED", "FINISHED", ...
        bool is_satisfied;
        contacts_t contacts;
        variable_id_to_points_t ids;

        std::string to_string()
        {
            json::value result;
            result[U("status")] = json::value(utility::conversions::to_string_t(status_code));
            result[U("is_satisfied")] = json::value(is_satisfied);
            result[U("contacts")] = json_contants();
            result[U("ids")] = json_ids();

            try
            {
                utility::stringstream_t stream;

                std::string resultingString = utility::conversions::to_utf8string(result.serialize());

                return resultingString;
                // string_t x = result.as_string();
            }
            catch(const std::exception& exc)
            {
                // catch anything thrown within try block that derives from std::exception
                std::string xq = exc.what();
            }
            return utility::conversions::to_utf8string(result.as_string());
        }

        json::value json_contants()
        {
            json::value result = json::value::array();
            for(int i = 0, len_i = contacts.size(); i < len_i; ++i)
            {
                json::value inner_array = json::value::array();
                for(int j = 0, len_j = contacts[i].size(); j < len_j; ++j)
                {
                    inner_array[j] = json::value(contacts[i][j] == true ? U("1") : U("0"));
                }
                result[i] = inner_array;
            }

            return result;
        }

        json::value json_ids()
        {
            json::value result = json::value::array();
            for(int i = 0, len = ids.size(); i < len; ++i)
            {
                json::value inner_array = json::value::array();
                for(int j = 0, len_j = ids[i].size(); j < len_j; ++j)
                {
                    inner_array[j] = json::value(ids[i][j] == true ? U("1") : U("0"));
                }
                result[i] = inner_array;
            }

            return result;
        }
    };

    auto extract_formula_refiners(std::string formula_filters) -> formula_mgr::formula_refiners;

    void remove_non_aciteve();

    std::mutex op_id_to_ctx_mutex_;
    std::unordered_map<std::string, pplx::cancellation_token_source> op_id_to_cts_;
    std::unordered_set<std::string> active_tasks;
    std::unordered_map<std::string, task_result> op_id_to_task_result;
    std::thread looping_thread;
    std::string CLIENT_DIR = "../client/dist";
};
