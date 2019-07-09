#include "catch/catch.hpp"

#include "library.h"
#include "nlohmann_json/json.hpp"

using json = nlohmann::json;

auto variables_check = [](const json& formula_json, const variables_set_t& expected_variables) {
    auto copy_f = formula_json;
    formula_mgr f;
    CHECK(f.build(copy_f));

    variables_set_t variables;
    f.get_variables(variables);
    CHECK(variables == expected_variables);
};

TEST_CASE("variables_check 1", "[variables_check]")
{
    // C(a, b) | ~C(a,b)
    variables_check(
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
        {"a", "b"});
}

TEST_CASE("variables_check 2", "[variables_check]")
{
    // C(a, a)
    variables_check(
        R"({
            "name": "contact",
            "value": [
               {
                  "name": "string",
                  "value": "a"
               },
               {
                  "name": "string",
                  "value": "a"
               }
            ]
        })"_json,
        {"a"});
}

TEST_CASE("variables_check 2.1", "[variables_check]")
{
    // C(axxa, axxa)
    variables_check(
        R"({
            "name": "contact",
            "value": [
               {
                  "name": "string",
                  "value": "axxa"
               },
               {
                  "name": "string",
                  "value": "axxa"
               }
            ]
        })"_json,
        {"axxa"});
}

TEST_CASE("variables_check 2.2", "[variables_check]")
{
    // <=(axxa, axxa)
    variables_check(
        R"({
            "name": "less",
            "value": [
               {
                  "name": "string",
                  "value": "axxa"
               },
               {
                  "name": "string",
                  "value": "axxa"
               }
            ]
        })"_json,
        {"axxa"});
}

TEST_CASE("variables_check 2.3", "[variables_check]")
{
    // C(a, b)
    variables_check(
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
        {"a", "b"});
}

TEST_CASE("variables_check 3", "[variables_check]")
{
    // C(a, b) & ~C(a,b)
    variables_check(
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
        {"a", "b"});
}

TEST_CASE("variables_check 4", "[variables_check]")
{
    // (C(a, b) | <=(a, b)) | ~C(a,b)
    variables_check(
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
        {"a", "b"});
}

TEST_CASE("variables_check 5", "[variables_check]")
{
    // (C(a, b) | <=(a, b)) | (~C(a,b) & <=(a,b))
    variables_check(
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
                  "name": "conjunction",
                  "value": [
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
               }
            ]
        })"_json,
        {"a", "b"});
}

TEST_CASE("variables_check 6", "[variables_check]")
{
    // C(a, b) & ~C(a, c)
    variables_check(
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
        {"a", "b", "c"});
}

TEST_CASE("variables_check 7", "[variables_check]")
{
    // (C(a, b) | ~C(a, b)) & (C(a,b) | ~C(a,b))
    variables_check(
        R"({
            "name": "conjunction",
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
               },
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
               }
            ]
        })"_json,
        {"a", "b"});
}

TEST_CASE("variables_check 8", "[variables_check]")
{
    // (C(a, b) | ~C(a, b)) & (C(a,b) | ~C(a,c))
    variables_check(
        R"({
           "name": "conjunction",
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
              },
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
              }
           ]
        })"_json,
        {"a", "b", "c"});
}

TEST_CASE("variables_check 9", "[variables_check]")
{
    // C(a, *b) & ~C(a, b + h)
    variables_check(
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
                        "name": "Tstar",
                        "value": {
                           "name": "string",
                           "value": "b"
                        }
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
                           "name": "Tand",
                           "value": [
                              {
                                 "name": "string",
                                 "value": "b"
                              },
                              {
                                 "name": "string",
                                 "value": "h"
                              }
                           ]
                        }
                     ]
                  }
               }
            ]
        })"_json,
        {"a", "b", "h"});
}

TEST_CASE("variables_check 10", "[variables_check]")
{
    // C(a - c, (*b) + h) & ~C(a - c, (*b) + h)
    variables_check(
        R"({
            "name": "conjunction",
            "value": [
               {
                  "name": "contact",
                  "value": [
                     {
                        "name": "Tor",
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
                     },
                     {
                        "name": "Tand",
                        "value": [
                           {
                              "name": "Tstar",
                              "value": {
                                 "name": "string",
                                 "value": "b"
                              }
                           },
                           {
                              "name": "string",
                              "value": "h"
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
                           "name": "Tor",
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
                        },
                        {
                           "name": "Tand",
                           "value": [
                              {
                                 "name": "Tstar",
                                 "value": {
                                    "name": "string",
                                    "value": "b"
                                 }
                              },
                              {
                                 "name": "string",
                                 "value": "h"
                              }
                           ]
                        }
                     ]
                  }
               }
            ]
        })"_json,
        {"a", "b", "c", "h"});
}

TEST_CASE("variables_check 11", "[variables_check]")
{
    // (C(a, b) & (C(f, h) & C(j, k))) & ~((C(a, b) & C(f, h)) & C(j, k)){ "a", "b", "f", "h", "j", "k" }
    variables_check(
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
                       "name": "conjunction",
                       "value": [
                          {
                             "name": "contact",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "f"
                                },
                                {
                                   "name": "string",
                                   "value": "h"
                                }
                             ]
                          },
                          {
                             "name": "contact",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "j"
                                },
                                {
                                   "name": "string",
                                   "value": "k"
                                }
                             ]
                          }
                       ]
                    }
                 ]
              },
              {
                 "name": "negation",
                 "value": {
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
                                "name": "contact",
                                "value": [
                                   {
                                      "name": "string",
                                      "value": "f"
                                   },
                                   {
                                      "name": "string",
                                      "value": "h"
                                   }
                                ]
                             }
                          ]
                       },
                       {
                          "name": "contact",
                          "value": [
                             {
                                "name": "string",
                                "value": "j"
                             },
                             {
                                "name": "string",
                                "value": "k"
                             }
                          ]
                       }
                    ]
                 }
              }
           ]
        })"_json,
        {"a", "b", "f", "h", "j", "k"});
}

TEST_CASE("variables_check 12", "[variables_check]")
{
    // (C(a, b) & (C(f, h) & C(j, k))) & ~((C(a, b) & C(f, h)) & C(j, Fxa))
    variables_check(
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
                       "name": "conjunction",
                       "value": [
                          {
                             "name": "contact",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "f"
                                },
                                {
                                   "name": "string",
                                   "value": "h"
                                }
                             ]
                          },
                          {
                             "name": "contact",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "j"
                                },
                                {
                                   "name": "string",
                                   "value": "k"
                                }
                             ]
                          }
                       ]
                    }
                 ]
              },
              {
                 "name": "negation",
                 "value": {
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
                                "name": "contact",
                                "value": [
                                   {
                                      "name": "string",
                                      "value": "f"
                                   },
                                   {
                                      "name": "string",
                                      "value": "h"
                                   }
                                ]
                             }
                          ]
                       },
                       {
                          "name": "contact",
                          "value": [
                             {
                                "name": "string",
                                "value": "j"
                             },
                             {
                                "name": "string",
                                "value": "Fxa"
                             }
                          ]
                       }
                    ]
                 }
              }
           ]
        })"_json,
        {"a", "b", "f", "h", "j", "k", "Fxa"});
}
