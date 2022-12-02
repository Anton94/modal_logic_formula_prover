#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"

#include "library.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    // If some test fails, enable the loggers for more info
    // Use -h to see which arguments are supported by catch.
    // Run specific test by passing it's name as argument.

    set_trace_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    // set_info_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    // set_error_logger([](std::stringstream&& s) { std::cerr << "ERROR: " << s.rdbuf() << std::endl; });

    int result = Catch::Session().run(argc, argv);
    // potential teardown
    return result;
}
