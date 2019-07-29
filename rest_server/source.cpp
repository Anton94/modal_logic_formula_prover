#include <iostream>
#include <string>
#include <fstream>

#include "cmd_options/cxxopts.hpp"
#include "nlohmann_json/json.hpp"

#include "library.h"

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#include "microservice_controller.h"

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

using json = nlohmann::json;

// std::unique_ptr<microser

void on_init(const string_t& address)
{
	// Build our listener's URI from the configured address and the hard coded path /foo/bar

	uri_builder uri(address);
	uri.append_path(U("foo/bar"));

    auto addr = uri.to_uri().to_string();
    // g_http
}

int main(int argc, char* argv[])
{
    set_trace_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    set_info_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    set_error_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });

    error();
    try
    {
        cxxopts::Options options("FormulaProover", "One line description of MyProgram");

        options.add_options()
                ("h,help", "Print help")
                //("f,formula", "Formula in json fomrat", cxxopts::value<std::string>())
                //("i,input_file", "File containing a formula in json fomrat", cxxopts::value<std::string>());
            ;

        auto result = options.parse(argc, argv);

        if(result.count("help"))
        {
            info() << options.help();
            return 0;
        }



        auto fileStream = std::make_shared<ostream>();

        // Open stream to output file.
        pplx::task<void> requestTask = fstream::open_ostream(U("results.html")).then([=](ostream outFile)
        {
            *fileStream = outFile;

            // Create http_client to send the request.
            http_client client(U("http://www.bing.com/"));

            // Build request URI and start the request.
            uri_builder builder(U("/search"));
            builder.append_query(U("q"), U("cpprestsdk github"));
            return client.request(methods::GET, builder.to_string());
        })

            // Handle response headers arriving.
            .then([=](http_response response)
        {
            printf("Received response status code:%u\n", response.status_code());

            // Write response body into the file.
            return response.body().read_to_end(fileStream->streambuf());
        })

            // Close the file stream.
            .then([=](size_t)
        {
            return fileStream->close();
        });

        // Wait for all the outstanding I/O to complete and handle any exceptions
        try
        {
            requestTask.wait();
        }
        catch (const std::exception &e)
        {
            printf("Error exception:%s\n", e.what());
        }


    }
    catch(const cxxopts::OptionException& e)
    {
        error() << "arg error: " << e.what();
    }

    return 0;
}
