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

std::unique_ptr<microservice_controller> g_http;

void on_init(const string_t& address)
{
    // Build our listener's URI from the configured address and the hard coded path /foo/bar

    uri_builder uri(address);

    auto addr = uri.to_uri().to_string();
    g_http = std::unique_ptr<microservice_controller>(new microservice_controller(addr));
    g_http->open();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;

    return;
}

void on_shutdown()
{
    g_http->close().wait();
    return;
}

int main(int argc, char* argv[])
{
    //set_verbose_logger([](std::stringstream&& s) { std::cout << "Verbose: " << s.rdbuf() << std::endl; });
    //set_trace_logger([](std::stringstream&& s) { std::cout << "Trace: " << s.rdbuf() << std::endl; });
    set_info_logger([](std::stringstream&& s) { std::cout << "Info: " << s.rdbuf() << std::endl; });
    set_error_logger([](std::stringstream&& s) { std::cout << "Error: " << s.rdbuf() << std::endl; });

    error();
    try
    {
        cxxopts::Options options("FormulaProover", "One line description of MyProgram");

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

        on_init(address);
        std::cout << "Press q to exit:" << std::endl;

        char c;
        do
        {
            std::cin >> c;
        } while(std::cin.good() && c != 'q' && c != 'Q');

        std::string line;
        std::getline(std::cin, line);

        on_shutdown();
    }
    catch(const cxxopts::OptionException& e)
    {
        error() << "arg error: " << e.what();
    }

    return 0;
}
