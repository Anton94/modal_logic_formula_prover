#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "cmd_options/cxxopts.hpp"

#include "microservice_controller.h"
//#include <cpprest/filestream.h>

using namespace utility;              // Common utilities like string conversions
using namespace web;                  // Common features like URIs.
using namespace web::http;            // Common HTTP functionality
using namespace web::http::client;    // HTTP client features
using namespace concurrency::streams; // Asynchronous streams

using namespace std::chrono_literals;

namespace
{
static std::unique_ptr<microservice_controller> g_http;

void on_init(const string_t& address, size_t concurrent_tasks_limit, const std::chrono::milliseconds& task_run_time_limit)
{
    // Build our listener's URI from the configured address and the hard coded path /foo/bar

    uri_builder uri(address);

    auto addr = uri.to_uri().to_string();
    g_http = std::make_unique<microservice_controller>(addr, concurrent_tasks_limit, task_run_time_limit);
    g_http->open().wait();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;
}

//void on_shutdown()
//{
//    g_http->close().wait();
//}
}

int main(int argc, char* argv[])
{
    // set_verbose_logger([](std::stringstream&& s) { std::cout << "Verbose: " << s.rdbuf() << std::endl; });
    // set_trace_logger([](std::stringstream&& s) { std::cout << "Trace: " << s.rdbuf() << std::endl; });
    set_info_logger([](std::stringstream&& s) { std::cout << "Info: " << s.rdbuf() << std::endl; });
    set_error_logger([](std::stringstream&& s) { std::cout << "Error: " << s.rdbuf() << std::endl; });

    try
    {
        cxxopts::Options options("RestServer", "Server which provides the web resources and handles the heavy algortihm executions");

        options.add_options()("h,help", "Print help");

        auto result = options.parse(argc, argv);

        if(result.count("help"))
        {
            info() << options.help();
            return 0;
        }

        utility::string_t port = U("34567");
        utility::string_t address = U("http://localhost:");
        address.append(port);

        on_init(address, 5, 30000ms);

        while(true)
        {
            std::this_thread::sleep_for(1s);
            g_http->remove_non_active();
            g_http->print_info();
        }

        // on_shutdown();
    }
    catch(const cxxopts::OptionException& e)
    {
        error() << "arg error: " << e.what();
    }
    catch(const std::exception& e)
    {
        error() << e.what();
    }
    catch(...)
    {
        error() << "Unknown error received";
    }

    return 0;
}
