#include "catch/catch.hpp"

#include "library.h"
#include "nlohmann_json/json.hpp"

using json = nlohmann::json;

auto is_satisfiable = [](const json& formula_json, bool expected_result) {
    auto copy_f = formula_json;
    formula f;
    CHECK(f.build(copy_f));

    tableau t;
    CHECK(t.is_satisfiable(f) == expected_result);
};

TEST_CASE("satisfiable 1", "[satisfiability]")
{
    // C(a, b) | ~C(a,b)
    is_satisfiable(
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

TEST_CASE("satisfiable 2", "[satisfiability]")
{
    // C(a, b)
    is_satisfiable(
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

TEST_CASE("satisfiable 3", "[satisfiability]")
{
    // C(a, b) & ~C(a,b)
    is_satisfiable(
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

TEST_CASE("satisfiable 4", "[satisfiability]")
{
    // (C(a, b) | <=(a, b)) | ~C(a,b)
    is_satisfiable(
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

TEST_CASE("satisfiable 5", "[satisfiability]")
{
    // (C(a, b) | <=(a, b)) | (~C(a,b) & <=(a,b))
    is_satisfiable(
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
        true);
}

TEST_CASE("satisfiable 6", "[satisfiability]")
{
    // C(a, b) & ~C(a, c)
    is_satisfiable(
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

TEST_CASE("satisfiable 7", "[satisfiability]")
{
    // (C(a, b) | ~C(a, b)) & (C(a,b) | ~C(a,b))
    is_satisfiable(
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
        true);
}

TEST_CASE("satisfiable 8", "[satisfiability]")
{
    // (C(a, b) | ~C(a, b)) & (C(a,b) | ~C(a,c))
    is_satisfiable(
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
        true);
}

TEST_CASE("satisfiable 9", "[satisfiability]")
{
    // C(a, *b) & ~C(a, b + h)
    is_satisfiable(
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
        true);
}

TEST_CASE("satisfiable 10", "[satisfiability]")
{
    // C(a - c, (*b) + h) & ~C(a - c, (*b) + h)
    is_satisfiable(
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
        false);
}

TEST_CASE("satisfiable 11", "[satisfiability]")
{
    // (C(a, b) & (C(f, h) & C(j, k))) & ~((C(a, b) & C(f, h)) & C(j, k))
    is_satisfiable(
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
        false);
}

TEST_CASE("satisfiable 12", "[satisfiability]")
{
    // (C(a, b) & (C(f, h) & C(j, k))) & ~((C(a, b) & C(f, h)) & C(j, F))
    is_satisfiable(
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
                                "value": "F"
                             }
                          ]
                       }
                    ]
                 }
              }
           ]
        })"_json,
        true);
}
