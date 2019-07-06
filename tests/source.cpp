#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"

#include "library.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    //set_trace_logger([](const std::string& s) { std::cout << s << std::endl; });
    set_info_logger([](const std::string& s) { std::cout << s << std::endl; });
    set_error_logger([](const std::string& s) { std::cerr << s << std::endl; });

    int result = Catch::Session().run(argc, argv);
    // potential teardown
    return result;
}
