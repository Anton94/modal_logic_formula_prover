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

void on_init(const string_t& address, size_t concurrent_tasks_limit, const std::chrono::milliseconds& task_run_time_limit, size_t connected_model_max_used_variables)
{
    // Build our listener's URI from the configured address and the hard coded path /foo/bar

    uri_builder uri(address);

    auto addr = uri.to_uri().to_string();
    g_http = std::make_unique<microservice_controller>(addr, concurrent_tasks_limit, task_run_time_limit, connected_model_max_used_variables);
    g_http->open().wait();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;
    ucout << utility::string_t(U("Open: ")) << addr << utility::string_t(U("index.html")) << std::endl;
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
        utility::string_t port = U("34567");
        utility::string_t address = U("http://localhost:");
        size_t concurrent_tasks_limit = 8ul;
        std::chrono::milliseconds task_run_time_limit = 300s;
        size_t connected_model_max_used_variables = 10ul;

        cxxopts::Options options("RestServer", "Server which provides the web resources and handles the heavy algortihm executions");

        options.add_options()("h,help", "Print help")
                             ("p,port", "Port. Default is " + utility::conversions::to_utf8string(port), cxxopts::value<size_t>())
                             ("a,address", "Address. Default is " + utility::conversions::to_utf8string(address), cxxopts::value<std::string>())
                             ("c,concurrent_tasks_limit", "Concurrent tasks limit, i.e. the max number of tasks which can be ran in the same time. Default is " + std::to_string(concurrent_tasks_limit), cxxopts::value<size_t>())
                             ("t,task_run_time_limit", "The time limit of the task's execution. Default is " + std::to_string(task_run_time_limit.count()) + "ms", cxxopts::value<size_t>())
                             ("u,connected_model_max_used_variables", "The max number of used variables when tring to create a connected model. Default is " + std::to_string(connected_model_max_used_variables), cxxopts::value<size_t>());

        auto result = options.parse(argc, argv);
        std::cout << "ARGS: ";
        for(const auto& arg : result.arguments())
        {
            std::cout << "[" << arg.key() << " = " << arg.value() << "] ";
        }
        std::cout << std::endl;

        if(result.count("help"))
        {
            info() << options.help();
            return 0;
        }
        if(result.count("port"))
        {
            port = utility::conversions::to_string_t(std::to_string(result["port"].as<size_t>()));
        }
        if(result.count("address"))
        {
            address = utility::conversions::to_string_t(result["address"].as<std::string>());
        }
        if(result.count("concurrent_tasks_limit"))
        {
            concurrent_tasks_limit = result["concurrent_tasks_limit"].as<size_t>();
        }
        if(result.count("task_run_time_limit"))
        {
            task_run_time_limit = std::chrono::milliseconds(result["task_run_time_limit"].as<size_t>());
        }
        if(result.count("connected_model_max_used_variables"))
        {
            connected_model_max_used_variables = result["connected_model_max_used_variables"].as<size_t>();
        }

        address.append(port);

        on_init(address, concurrent_tasks_limit, task_run_time_limit, connected_model_max_used_variables);

        while(true)
        {
            std::this_thread::sleep_for(10s);
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
