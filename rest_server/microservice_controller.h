#pragma once

#include "library.h"

#include <cpprest/asyncrt_utils.h>
#include <cpprest/containerstream.h>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class microservice_controller
{
public:
    microservice_controller()
    {
    }
    microservice_controller(utility::string_t url);

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

	pplx::cancellation_token_source cts_;
    std::string CLIENT_DIR = "../client/dist";
};
