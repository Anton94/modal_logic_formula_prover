#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"

#include "library.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    // If some test fails, enable the loggers for more info
    set_trace_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    set_info_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    set_error_logger([](std::stringstream&& s) { std::cerr << s.rdbuf() << std::endl; });

    int result = Catch::Session().run(argc, argv);
    // potential teardown
    return result;
}
