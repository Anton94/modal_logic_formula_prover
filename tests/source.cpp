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
    set_error_logger([](std::stringstream&& s) { std::cerr << "ERROR: " << s.rdbuf() << std::endl; });

    int result = Catch::Session().run(argc, argv);
    // potential teardown
    return result;
}

// TODO make common file for such tests
TEST_CASE("move assignment of formula_mgr", "[run_with_sanitizer]")
{
    // (C(a, b) & ~C(c, 0)) & <=(1, 0)
    auto complex_formula_json =
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "contact",
                       "value": [
                          {
                             "name": "string",
                             "value": "a"
                          },
                          {
                             "name": "string",
                             "value": "b"
                          }
                       ]
                    },
                    {
                       "name": "negation",
                       "value": {
                          "name": "contact",
                          "value": [
                             {
                                "name": "string",
                                "value": "c"
                             },
                             {
                                "name": "string",
                                "value": "0"
                             }
                          ]
                       }
                    }
                 ]
              },
              {
                 "name": "equal0",
                 "value": {
                    "name": "Tand",
                    "value": [
                       {
                          "name": "constant_1"
                       },
                       {
                          "name": "Tstar",
                          "value": {
                              "name": "constant_0"
                          }
                       }
                    ]
                 }
              }
           ]
        })"_json;
    formula_mgr mgr;

    mgr.build(complex_formula_json);

    // C(a, b)
    auto simple_formula_json =
        R"({
        "name": "contact",
        "value": [
           {
              "name": "string",
              "value": "a"
           },
           {
              "name": "string",
              "value": "b"
           }
        ]
    })"_json;
    auto simple_formula_json_copy = simple_formula_json;

    formula_mgr mgr2;
    mgr2.build(simple_formula_json);
    mgr2 = std::move(mgr);

    formula_mgr mgr3 = std::move(mgr2);
}
