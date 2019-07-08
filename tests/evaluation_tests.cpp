#include "catch/catch.hpp"

#include "library.h"
#include "nlohmann_json/json.hpp"

using json = nlohmann::json;

auto check_for_satisfiable_evaluation = [](const json& formula_json, bool expected_result) {
    auto copy_f = formula_json;
    formula_mgr f;
    CHECK(f.build(copy_f));

    CHECK(f.brute_force_evaluate() == expected_result);
};

TEST_CASE("satisfiable evaluation 1", "[brute force evaluation]")
{
    // C(a, b) | ~C(a,b)
    check_for_satisfiable_evaluation(
        R"({
            "name": "disjunction",
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
                           "value": "a"
                        },
                        {
                           "name": "string",
                           "value": "b"
                        }
                     ]
                  }
               }
            ]
        })"_json,
        true);
}

TEST_CASE("satisfiable evaluation 2", "[brute force evaluation]")
{
    // C(a, b)
    check_for_satisfiable_evaluation(
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
        })"_json,
        true);
}

TEST_CASE("satisfiable evaluation 3", "[brute force evaluation]")
{
    // C(a, b) & ~C(a,c)
    check_for_satisfiable_evaluation(
        R"({
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
                           "value": "a"
                        },
                        {
                           "name": "string",
                           "value": "c"
                        }
                     ]
                  }
               }
            ]
        })"_json,
        true);
}

TEST_CASE("satisfiable evaluation 4", "[brute force evaluation]")
{
    // C(a, b) & ~C(a,b)
    check_for_satisfiable_evaluation(
        R"({
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
                           "value": "a"
                        },
                        {
                           "name": "string",
                           "value": "b"
                        }
                     ]
                  }
               }
            ]
        })"_json,
        false);
}
