#include "catch/catch.hpp"

#include "library.h"
#include "nlohmann_json/json.hpp"

using json = nlohmann::json;

auto is_tautology = [](const json& formula_json, bool expected_result) {
    auto copy_f = formula_json;
    formula f;
    CHECK(f.build(copy_f));

    tableau t;
    CHECK(t.is_tautology(f) == expected_result);
};

TEST_CASE("tautology 1", "[tautology]")
{
    // C(a, b) | ~C(a,b)
    is_tautology(
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

TEST_CASE("tautology 2", "[tautology]")
{
    // C(a, b)
    is_tautology(
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
        false);
}

TEST_CASE("tautology 3", "[tautology]")
{
    // C(a, b) & ~C(a,b)
    is_tautology(
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

TEST_CASE("tautology 4", "[tautology]")
{
    // (C(a, b) | <=(a, b)) | ~C(a,b)
    is_tautology(
        R"({
            "name": "disjunction",
            "value": [
               {
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
                        "name": "less",
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
