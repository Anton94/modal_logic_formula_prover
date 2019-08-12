#include "catch/catch.hpp"

#include "library.h"
#include "nlohmann_json/json.hpp"

using json = nlohmann::json;

auto check_for_satisfiable_evaluation = [](const json& formula_json, bool expected_result) {
    auto copy_f = formula_json;
    formula_mgr f;
    CHECK(f.build(copy_f));

    variable_to_evaluation_map_t out_evaluations;
    CHECK(f.brute_force_evaluate(out_evaluations) == expected_result);
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

TEST_CASE("satisfiable evaluation with constant 0", "[brute force evaluation]")
{
    // C(1, b) & ~C(a, 0)
    check_for_satisfiable_evaluation(
        R"({
            "name": "conjunction",
            "value": [
               {
                  "name": "contact",
                  "value": [
                     {
                        "name": "1"
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
                           "name": "0"
                        }
                     ]
                  }
               }
            ]
        })"_json,
        true);
}

TEST_CASE("satisfiable evaluation with constant 1", "[brute force evaluation]")
{
    // (C(a, b) & ~C(c, 0)) & <=(1, 0)
    check_for_satisfiable_evaluation(
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
        })"_json,
        false);
}

TEST_CASE("satisfiable evaluation with constant 2", "[brute force evaluation]")
{
    // (C(a, b) & ~C(c, 1)) & <=(1, a)
    check_for_satisfiable_evaluation(
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
                                "name": "1"
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
                              "name": "string",
                              "value": "a"
                          }
                       }
                    ]
                 }
              }
           ]
        })"_json,
        true);
}

TEST_CASE("satisfiable evaluation with constant 3", "[brute force evaluation]")
{
    // (C(a, b) & ~C(c, 1)) & T
    check_for_satisfiable_evaluation(
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
                                "name": "1"
                             }
                          ]
                       }
                    }
                 ]
              },
              {
                 "name": "T"
              }
           ]
        })"_json,
        true);
}

TEST_CASE("satisfiable evaluation with constant 4", "[brute force evaluation]")
{
    // (C(a, b) & ~C(c, 1)) & F
    check_for_satisfiable_evaluation(
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
                                "name": "1"
                             }
                          ]
                       }
                    }
                 ]
              },
              {
                 "name": "F"
              }
           ]
        })"_json,
        false);
}

TEST_CASE("satisfiable evaluation with constant 5", "[brute force evaluation]")
{
    // (C(a, b) & ~C(c, 1)) | F
    check_for_satisfiable_evaluation(
        R"({
           "name": "disjunction",
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
                                "name": "1"
                             }
                          ]
                       }
                    }
                 ]
              },
              {
                 "name": "F"
              }
           ]
        })"_json,
        true);
}

TEST_CASE("satisfiable evaluation with constant 6", "[brute force evaluation]")
{
    // (C(0, b) & ~C(c, 1)) | F
    check_for_satisfiable_evaluation(
        R"({
           "name": "disjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "contact",
                       "value": [
                          {
                               "name": "0"
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
                                "name": "1"
                             }
                          ]
                       }
                    }
                 ]
              },
              {
                 "name": "F"
              }
           ]
        })"_json,
        false);
}

TEST_CASE("satisfiable evaluation with constant 7", "[brute force evaluation]")
{
    // (C(0, b) & ~C(c, 1)) | T
    check_for_satisfiable_evaluation(
        R"({
           "name": "disjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "contact",
                       "value": [
                          {
                             "name": "0"
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
                                "name": "1"
                             }
                          ]
                       }
                    }
                 ]
              },
              {
                 "name": "T"
              }
           ]
        })"_json,
        true);
}
