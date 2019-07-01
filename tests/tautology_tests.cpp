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
    is_tautology(
        R"({ "name": "or",
             "value": [
                 {
                     "name": "or",
                     "value": [
                         {
                             "name": "le",
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
                             "name": "neg",
                             "value": {
                                 "name": "le",
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
                 },
                 {
                     "name": "C",
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
        })"_json,
        true);
}

TEST_CASE("tautology 2", "[tautology]")
{
    is_tautology(
        R"({ "name": "or",
             "value": [
                 {
                     "name": "or",
                     "value": [
                         {
                             "name": "le",
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
                             "name": "neg",
                             "value": {
                                 "name": "le",
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
                 },
                 {
                     "name": "C",
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
        })"_json,
        true);
}
