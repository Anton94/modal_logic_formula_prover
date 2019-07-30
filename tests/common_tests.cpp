#include "catch/catch.hpp"

#include "library.h"
#include "nlohmann_json/json.hpp"

using json = nlohmann::json;

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
                                "name": "0"
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
                          "name": "1"
                       },
                       {
                          "name": "Tstar",
                          "value": {
                              "name": "0"
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
