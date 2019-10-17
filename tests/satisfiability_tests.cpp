#include "catch/catch.hpp"

#include "library.h"
#include "nlohmann_json/json.hpp"

using json = nlohmann::json;

auto is_satisfiable = [](const json& formula_json, bool has_satisfiable_model, bool has_satisfiable_connected_model,
                         bool run_bruteforce = true) {
    auto copy_f = formula_json;
    formula_mgr f;
    CHECK(f.build(copy_f));

    model m;
    CHECK(f.is_satisfiable(m) == has_satisfiable_model);

    if(has_satisfiable_model)
    {
        CHECK(f.is_model_satisfiable(m));
    }

    slow_model slow_m;
    CHECK(f.is_satisfiable(slow_m) == has_satisfiable_model);

    if(has_satisfiable_model)
    {
        CHECK(f.is_model_satisfiable(slow_m));
    }

    connected_model connected_m;
    CHECK(f.is_satisfiable(connected_m) == has_satisfiable_connected_model);
    if(has_satisfiable_connected_model)
    {
        CHECK(f.is_model_satisfiable(slow_m));
    }

    if(run_bruteforce)
    {
        basic_bruteforce_model model;
        CHECK(f.brute_force_evaluate_with_points_count(model) == has_satisfiable_model);
    }
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
        true, true);
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
        true, true);
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
        false, false);
}

TEST_CASE("satisfiable 3.1", "[satisfiability]")
{
    // C(a, b) & ~C(a,c)
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
        true, true);
}

TEST_CASE("satisfiable evaluation with constant 0", "[satisfiability]")
{
    // C(1, b) & ~C(a, 0)
    is_satisfiable(
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
        true, true);
}

TEST_CASE("satisfiable evaluation with constant 1", "[satisfiability]")
{
    // (C(a, b) & ~C(c, 0)) & <=(1, 0)
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
        false, false);
}

TEST_CASE("satisfiable evaluation with constant 1.1", "[satisfiability]")
{
    // (C(a, b) & ~C(c, 0)) & <=(0, 1)
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
                          "name": "0"
                       },
                       {
                          "name": "Tstar",
                          "value": {
                              "name": "1"
                          }
                       }
                    ]
                 }
              }
           ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable evaluation with constant 2", "[satisfiability]")
{
    // (C(a, b) & ~C(c, 1)) & <=(1, a)
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
        true, true);
}

TEST_CASE("satisfiable evaluation with constant 3", "[satisfiability]")
{
    // (C(a, b) & ~C(c, 1)) & T
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
        true, true);
}

TEST_CASE("satisfiable evaluation with constant 4", "[satisfiability]")
{
    // (C(a, b) & ~C(c, 1)) & F
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
        false, false);
}

TEST_CASE("satisfiable evaluation with constant 5", "[satisfiability]")
{
    // (C(a, b) & ~C(c, 1)) | F
    is_satisfiable(
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
        true, true);
}

TEST_CASE("satisfiable evaluation with constant 6", "[satisfiability]")
{
    // (C(0, b) & ~C(c, 1)) | F
    is_satisfiable(
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
        false, false);
}

TEST_CASE("satisfiable evaluation with constant 7", "[satisfiability]")
{
    // (C(0, b) & ~C(c, 1)) | T
    is_satisfiable(
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
        true, true);
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
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
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
                          "name": "string",
                          "value": "b"
                       }
                    ]
                 }
              }
           ]
         })"_json,
        true, true);
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
                        "name": "equal0",
                        "value": {
                           "name": "Tand",
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
                        }
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
                        "name": "equal0",
                        "value": {
                           "name": "Tand",
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
                        }
                     }
                  ]
               }
            ]
        })"_json,
        true, true);
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
        true, true);
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
        true, true);
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
        true, true);
}

TEST_CASE("satisfiable 9", "[satisfiability]")
{
    // C(a, -b) & ~C(a, b + h)
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
        true, true);
}

TEST_CASE("satisfiable 10", "[satisfiability]")
{
    // C(a * c, (-b) + h) & ~C(a * c, (-b) + h)
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
        false, false);
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
        false, false, false);
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
        true, true, false);
}

TEST_CASE("satisfiable 13", "[satisfiability]")
{
    // <=(a,b) & <=(b,a)
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "equal0",
                 "value": {
                    "name": "Tand",
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
                 }
              },
              {
                 "name": "equal0",
                 "value": {
                    "name": "Tand",
                    "value": [
                       {
                          "name": "string",
                          "value": "b"
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
        true, true);
}

TEST_CASE("satisfiable 14", "[satisfiability]")
{
    // <=(a,b) & ~(<=(b,a))
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "equal0",
                 "value": {
                    "name": "Tand",
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
                 }
              },
              {
                 "name": "negation",
                 "value": {
                    "name": "equal0",
                    "value": {
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "b"
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
              }
           ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable 15", "[satisfiability]")
{
    // <=(a,b) & ~(<=(a,b))
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "equal0",
                 "value": {
                    "name": "Tand",
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
                 }
              },
              {
                 "name": "negation",
                 "value": {
                    "name": "equal0",
                    "value": {
                       "name": "Tand",
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
                    }
                 }
              }
           ]
        })"_json,
        false, false);
}

TEST_CASE("satisfiable 16", "[satisfiability]")
{
    // (<=(a,b) & (~(<=(b,a)))) | (<=(a,b))
    is_satisfiable(
        R"({
           "name": "disjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
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
                       }
                    },
                    {
                       "name": "negation",
                       "value": {
                          "name": "equal0",
                          "value": {
                             "name": "Tand",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "b"
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
                    }
                 ]
              },
              {
                 "name": "equal0",
                 "value": {
                    "name": "Tand",
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
                 }
              }
           ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable 17", "[satisfiability]")
{
    // (<=(a,b) & (~(<=(a,b)))) | (C(a,b) & (~C(a,b)))
    is_satisfiable(
        R"({
           "name": "disjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
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
                       }
                    },
                    {
                       "name": "negation",
                       "value": {
                          "name": "equal0",
                          "value": {
                             "name": "Tand",
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
                          }
                       }
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
        false, false);
}

TEST_CASE("satisfiable 18", "[satisfiability]")
{
    // (<=(a,b) & (~(<=(a,b)))) | (C(a,b) & (~C(b,b))))
    is_satisfiable(
        R"({
           "name": "disjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
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
                       }
                    },
                    {
                       "name": "negation",
                       "value": {
                          "name": "equal0",
                          "value": {
                             "name": "Tand",
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
                          }
                       }
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
                                "value": "b"
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
        false, false);
}

TEST_CASE("satisfiable 18.1", "[satisfiability]")
{
    // (<=(a,b) & (~(<=(a,b)))) | (C(a,b) & (~C(-b,b)))
    is_satisfiable(
        R"({
           "name": "disjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
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
                       }
                    },
                    {
                       "name": "negation",
                       "value": {
                          "name": "equal0",
                          "value": {
                             "name": "Tand",
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
                          }
                       }
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
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "b"
                                }
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
        true, true);
}

TEST_CASE("satisfiable 18.2", "[satisfiability]")
{
    // (C(a,b) & ~C(b,b)) | (C(a,b) & ~C(-b,b))
    is_satisfiable(
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
                                "value": "b"
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
                                 "name": "Tstar",
                                 "value": {
                                    "name": "string",
                                    "value": "b"
                                 }
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
        true, true);
}

TEST_CASE("satisfiable 19", "[satisfiability]")
{
    // ~(C([x * -z], [b * -k]) | C([x * -z], c)) & (~C(-b,b) & (~<=(x, z) & ~<=(b, k)))
    is_satisfiable(
        R"({
            "name": "conjunction",
            "value": [
               {
                  "name": "negation",
                  "value": {
                     "name": "disjunction",
                     "value": [
                        {
                           "name": "contact",
                           "value": [
                              {
                                 "name": "Tand",
                                 "value": [
                                    {
                                       "name": "string",
                                       "value": "x"
                                    },
                                    {
                                       "name": "Tstar",
                                       "value": {
                                          "name": "string",
                                          "value": "z"
                                       }
                                    }
                                 ]
                              },
                              {
                                 "name": "Tand",
                                 "value": [
                                    {
                                       "name": "string",
                                       "value": "b"
                                    },
                                    {
                                       "name": "Tstar",
                                       "value": {
                                          "name": "string",
                                          "value": "k"
                                       }
                                    }
                                 ]
                              }
                           ]
                        },
                        {
                           "name": "contact",
                           "value": [
                              {
                                 "name": "Tand",
                                 "value": [
                                    {
                                       "name": "string",
                                       "value": "x"
                                    },
                                    {
                                       "name": "Tstar",
                                       "value": {
                                          "name": "string",
                                          "value": "z"
                                       }
                                    }
                                 ]
                              },
                              {
                                 "name": "string",
                                 "value": "c"
                              }
                           ]
                        }
                     ]
                  }
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
                                 "name": "Tstar",
                                 "value": {
                                    "name": "string",
                                    "value": "b"
                                 }
                              },
                              {
                                 "name": "string",
                                 "value": "b"
                              }
                           ]
                        }
                     },
                     {
                        "name": "conjunction",
                        "value": [
                           {
                              "name": "negation",
                              "value": {
                                 "name": "equal0",
                                 "value": {
                                    "name": "Tand",
                                    "value": [
                                       {
                                          "name": "string",
                                          "value": "x"
                                       },
                                       {
                                          "name": "Tstar",
                                          "value": {
                                             "name": "string",
                                             "value": "z"
                                          }
                                       }
                                    ]
                                 }
                              }
                           },
                           {
                              "name": "negation",
                              "value": {
                                 "name": "equal0",
                                 "value": {
                                    "name": "Tand",
                                    "value": [
                                       {
                                          "name": "string",
                                          "value": "b"
                                       },
                                       {
                                          "name": "Tstar",
                                          "value": {
                                             "name": "string",
                                             "value": "k"
                                          }
                                       }
                                    ]
                                 }
                              }
                           }
                        ]
                     }
                  ]
               }
            ]
        })"_json,
        true, true, false);
}

TEST_CASE("satisfiable evaluation of path 1", "[satisfiability]")
{
    // ~C(x * -z,b) & (~C(-b,b) & ~<=(x, z))
    is_satisfiable(
        R"({
            "name": "conjunction",
            "value": [
               {
                  "name": "negation",
                  "value": {
                     "name": "contact",
                     "value": [
                        {
                           "name": "Tand",
                           "value": [
                              {
                                 "name": "string",
                                 "value": "x"
                              },
                              {
                                 "name": "Tstar",
                                 "value": {
                                    "name": "string",
                                    "value": "z"
                                 }
                              }
                           ]
                        },
                        {
                           "name": "string",
                           "value": "b"
                        }
                     ]
                  }
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
                                 "name": "Tstar",
                                 "value": {
                                    "name": "string",
                                    "value": "b"
                                 }
                              },
                              {
                                 "name": "string",
                                 "value": "b"
                              }
                           ]
                        }
                     },
                     {
                        "name": "negation",
                        "value": {
                           "name": "equal0",
                           "value": {
                              "name": "Tand",
                              "value": [
                                 {
                                    "name": "string",
                                    "value": "x"
                                 },
                                 {
                                    "name": "Tstar",
                                    "value": {
                                       "name": "string",
                                       "value": "z"
                                    }
                                 }
                              ]
                           }
                        }
                     }
                  ]
               }
            ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable evaluation of path 2", "[satisfiability]")
{
    // ~C(x * -z, (b * -k) + c) & (~C(-b,b) & (~<=(x, z) & ~<=(b, k)))
    is_satisfiable(
        R"({
            "name": "conjunction",
            "value": [
               {
                  "name": "negation",
                  "value": {
                     "name": "contact",
                     "value": [
                        {
                           "name": "Tand",
                           "value": [
                              {
                                 "name": "string",
                                 "value": "x"
                              },
                              {
                                 "name": "Tstar",
                                 "value": {
                                    "name": "string",
                                    "value": "z"
                                 }
                              }
                           ]
                        },
                        {
                           "name": "Tor",
                           "value": [
                              {
                                 "name": "Tand",
                                 "value": [
                                    {
                                       "name": "string",
                                       "value": "b"
                                    },
                                    {
                                       "name": "Tstar",
                                       "value": {
                                          "name": "string",
                                          "value": "k"
                                       }
                                    }
                                 ]
                              },
                              {
                                 "name": "string",
                                 "value": "c"
                              }
                           ]
                        }
                     ]
                  }
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
                                 "name": "Tstar",
                                 "value": {
                                    "name": "string",
                                    "value": "b"
                                 }
                              },
                              {
                                 "name": "string",
                                 "value": "b"
                              }
                           ]
                        }
                     },
                     {
                        "name": "conjunction",
                        "value": [
                           {
                              "name": "negation",
                              "value": {
                                 "name": "equal0",
                                 "value": {
                                    "name": "Tand",
                                    "value": [
                                       {
                                          "name": "string",
                                          "value": "x"
                                       },
                                       {
                                          "name": "Tstar",
                                          "value": {
                                             "name": "string",
                                             "value": "z"
                                          }
                                       }
                                    ]
                                 }
                              }
                           },
                           {
                              "name": "negation",
                              "value": {
                                 "name": "equal0",
                                 "value": {
                                    "name": "Tand",
                                    "value": [
                                       {
                                          "name": "string",
                                          "value": "b"
                                       },
                                       {
                                          "name": "Tstar",
                                          "value": {
                                             "name": "string",
                                             "value": "k"
                                          }
                                       }
                                    ]
                                 }
                              }
                           }
                        ]
                     }
                  ]
               }
            ]
        })"_json,
        true, true, false);
}

TEST_CASE("satisfiable evaluation of path 3", "[satisfiability]")
{
    // ~C(x * -z, (b * -k) * c) & (~C(-b,b) & (~<=(x, z) & ~<=(b, k)))
    is_satisfiable(
        R"({
            "name": "conjunction",
            "value": [
               {
                  "name": "negation",
                  "value": {
                     "name": "contact",
                     "value": [
                        {
                           "name": "Tand",
                           "value": [
                              {
                                 "name": "string",
                                 "value": "x"
                              },
                              {
                                 "name": "Tstar",
                                 "value": {
                                    "name": "string",
                                    "value": "z"
                                 }
                              }
                           ]
                        },
                        {
                           "name": "Tand",
                           "value": [
                              {
                                 "name": "Tand",
                                 "value": [
                                    {
                                       "name": "string",
                                       "value": "b"
                                    },
                                    {
                                       "name": "Tstar",
                                       "value": {
                                          "name": "string",
                                          "value": "k"
                                       }
                                    }
                                 ]
                              },
                              {
                                 "name": "string",
                                 "value": "c"
                              }
                           ]
                        }
                     ]
                  }
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
                                 "name": "Tstar",
                                 "value": {
                                    "name": "string",
                                    "value": "b"
                                 }
                              },
                              {
                                 "name": "string",
                                 "value": "b"
                              }
                           ]
                        }
                     },
                     {
                        "name": "conjunction",
                        "value": [
                           {
                              "name": "negation",
                              "value": {
                                 "name": "equal0",
                                 "value": {
                                    "name": "Tand",
                                    "value": [
                                       {
                                          "name": "string",
                                          "value": "x"
                                       },
                                       {
                                          "name": "Tstar",
                                          "value": {
                                             "name": "string",
                                             "value": "z"
                                          }
                                       }
                                    ]
                                 }
                              }
                           },
                           {
                              "name": "negation",
                              "value": {
                                 "name": "equal0",
                                 "value": {
                                    "name": "Tand",
                                    "value": [
                                       {
                                          "name": "string",
                                          "value": "b"
                                       },
                                       {
                                          "name": "Tstar",
                                          "value": {
                                             "name": "string",
                                             "value": "k"
                                          }
                                       }
                                    ]
                                 }
                              }
                           }
                        ]
                     }
                  ]
               }
            ]
        })"_json,
        true, true, false);
}

TEST_CASE("satisfiable evaluation of path 4", "[satisfiability]")
{
    // (C(a,a) & C(d,d)) & ~C(a,d)
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
                              "value": "a"
                           }
                        ]
                     },
                     {
                        "name": "contact",
                        "value": [
                           {
                              "name": "string",
                              "value": "d"
                           },
                           {
                              "name": "string",
                              "value": "d"
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
                           "value": "d"
                        }
                     ]
                  }
               }
            ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable evaluation of path 5", "[satisfiability]")
{
    // ((C(a,a) & C(d,d)) & ~C(a,d)) & ((n * m) * -o = 0)
    is_satisfiable(
        R"({
            "name": "conjunction",
            "value": [
               {
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
                                    "value": "a"
                                 }
                              ]
                           },
                           {
                              "name": "contact",
                              "value": [
                                 {
                                    "name": "string",
                                    "value": "d"
                                 },
                                 {
                                    "name": "string",
                                    "value": "d"
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
                                 "value": "d"
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
                           "name": "Tand",
                           "value": [
                              {
                                 "name": "string",
                                 "value": "n"
                              },
                              {
                                 "name": "string",
                                 "value": "m"
                              }
                           ]
                        },
                        {
                           "name": "Tstar",
                           "value": {
                              "name": "string",
                              "value": "o"
                           }
                        }
                     ]
                  }
               }
            ]
        })"_json,
        true, true, false);
}

TEST_CASE("satisfiable evaluation of path 6", "[satisfiability]")
{
    // C(a,b) & C(b,c) & <=(a,b) & ~C(a,c)
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
                       "name": "contact",
                       "value": [
                          {
                             "name": "string",
                             "value": "b"
                          },
                          {
                             "name": "string",
                             "value": "c"
                          }
                       ]
                    }
                 ]
              },
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
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
                       }
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
        true, true);
}

TEST_CASE("satisfiable evaluation of path 7", "[satisfiability]")
{
    // C(a,c) & <=(a,b) & ~C(b,c)
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
                             "value": "c"
                          }
                       ]
                    },
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
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
                          "value": "b"
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
        false, false);
}

TEST_CASE("satisfiable evaluation of path 8", "[satisfiability]")
{
    // (C(a,a) & C(d,d)) & (~C(a,d) & ((m*n)*-o = 0)
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
                              "value": "a"
                           }
                        ]
                     },
                     {
                        "name": "contact",
                        "value": [
                           {
                              "name": "string",
                              "value": "d"
                           },
                           {
                              "name": "string",
                              "value": "d"
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
                                 "value": "d"
                              }
                           ]
                        }
                     },
                     {
                        "name": "equal0",
                        "value": {
                           "name": "Tand",
                           "value": [
                              {
                                 "name": "Tand",
                                 "value": [
                                    {
                                       "name": "string",
                                       "value": "n"
                                    },
                                    {
                                       "name": "string",
                                       "value": "m"
                                    }
                                 ]
                              },
                              {
                                 "name": "Tstar",
                                 "value": {
                                    "name": "string",
                                    "value": "o"
                                 }
                              }
                           ]
                        }
                     }
                  ]
               }
            ]
        })"_json,
        true, true, false);
}

TEST_CASE("satisfiable with constants 1", "[satisfiability]")
{
    // T
    is_satisfiable(
        R"({
            "name": "T"
        })"_json,
        true, true);
}

TEST_CASE("satisfiable with constants 2", "[satisfiability]")
{
    // F
    is_satisfiable(
        R"({
            "name": "F"
        })"_json,
        false, false);
}

TEST_CASE("satisfiable with constants 3", "[satisfiability]")
{
    // T & F
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "T"
              },
              {
                 "name": "F"
              }
           ]
        })"_json,
        false, false);
}

TEST_CASE("satisfiable with constants 4", "[satisfiability]")
{
    // F & F
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "F"
              },
              {
                 "name": "F"
              }
           ]
        })"_json,
        false, false);
}

TEST_CASE("satisfiable with constants 5", "[satisfiability]")
{
    // F | F
    is_satisfiable(
        R"({
           "name": "disjunction",
           "value": [
              {
                 "name": "F"
              },
              {
                 "name": "F"
              }
           ]
        })"_json,
        false, false);
}

TEST_CASE("satisfiable with constants 6", "[satisfiability]")
{
    // T | F
    is_satisfiable(
        R"({
           "name": "disjunction",
           "value": [
              {
                 "name": "T"
              },
              {
                 "name": "F"
              }
           ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable with constants 7", "[satisfiability]")
{
    // C(a, *b) & T
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
                 "name": "T"
              }
           ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable with constants 8", "[satisfiability]")
{
    // C(a, -b) & F
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
                 "name": "F"
              }
           ]
        })"_json,
        false, false);
}

TEST_CASE("satisfiable with contact rule 1", "[satisfiability]")
{
    // <=(a, b) & C(a * (-b), c)
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "equal0",
                 "value": {
                    "name": "Tand",
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
                 }
              },
              {
                 "name": "contact",
                 "value": [
                    {
                       "name": "Tand",
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
                       "name": "string",
                       "value": "c"
                    }
                 ]
              }
           ]
        })"_json,
        false, false);
}

TEST_CASE("satisfiable with contact rule 1.5", "[satisfiability]")
{
    // C(a * (-b), c) & <=(a, b)
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
                {
                   "name": "contact",
                   "value": [
                      {
                         "name": "Tand",
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
                         "name": "string",
                         "value": "c"
                      }
                   ]
                },
                {
                   "name": "equal0",
                   "value": {
                      "name": "Tand",
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
                   }
                }
           ]
        })"_json,
        false, false);
}

TEST_CASE("satisfiable with contact rule 2", "[satisfiability]")
{
    // <=(a, c) & C(a * (-b), c)
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "equal0",
                 "value": {
                    "name": "Tand",
                    "value": [
                       {
                          "name": "string",
                          "value": "a"
                       },
                       {
                          "name": "Tstar",
                          "value": {
                             "name": "string",
                             "value": "c"
                          }
                       }
                    ]
                 }
              },
              {
                 "name": "contact",
                 "value": [
                    {
                       "name": "Tand",
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
                       "name": "string",
                       "value": "c"
                    }
                 ]
              }
           ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable with contact rule 3", "[satisfiability]")
{
    // <=(a, c) & (C(a * (-c), d) & C(a * (-b), c))
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "equal0",
                 "value": {
                    "name": "Tand",
                    "value": [
                       {
                          "name": "string",
                          "value": "a"
                       },
                       {
                          "name": "Tstar",
                          "value": {
                             "name": "string",
                             "value": "c"
                          }
                       }
                    ]
                 }
              },
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "contact",
                       "value": [
                          {
                             "name": "Tand",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "a"
                                },
                                {
                                   "name": "Tstar",
                                   "value": {
                                      "name": "string",
                                      "value": "c"
                                   }
                                }
                             ]
                          },
                          {
                             "name": "string",
                             "value": "d"
                          }
                       ]
                    },
                    {
                       "name": "contact",
                       "value": [
                          {
                             "name": "Tand",
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
                             "name": "string",
                             "value": "c"
                          }
                       ]
                    }
                 ]
              }
           ]
        })"_json,
        false, false);
}

TEST_CASE("satisfiable with contact rule 4", "[satisfiability]")
{
    // <=(a, c) & (C(a * (-c), d) | C(a * (-b), c))
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "equal0",
                 "value": {
                    "name": "Tand",
                    "value": [
                       {
                          "name": "string",
                          "value": "a"
                       },
                       {
                          "name": "Tstar",
                          "value": {
                             "name": "string",
                             "value": "c"
                          }
                       }
                    ]
                 }
              },
              {
                 "name": "disjunction",
                 "value": [
                    {
                       "name": "contact",
                       "value": [
                          {
                             "name": "Tand",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "a"
                                },
                                {
                                   "name": "Tstar",
                                   "value": {
                                      "name": "string",
                                      "value": "c"
                                   }
                                }
                             ]
                          },
                          {
                             "name": "string",
                             "value": "d"
                          }
                       ]
                    },
                    {
                       "name": "contact",
                       "value": [
                          {
                             "name": "Tand",
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
                             "name": "string",
                             "value": "c"
                          }
                       ]
                    }
                 ]
              }
           ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable with contact rule 5", "[satisfiability]")
{
    // (<=(a, c) | <=(a,b)) & (C(a * (-c), d) | C(a * (-b), c))
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "disjunction",
                 "value": [
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
                          "value": [
                             {
                                "name": "string",
                                "value": "a"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "c"
                                }
                             }
                          ]
                       }
                    },
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
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
                             "name": "Tand",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "a"
                                },
                                {
                                   "name": "Tstar",
                                   "value": {
                                      "name": "string",
                                      "value": "c"
                                   }
                                }
                             ]
                          },
                          {
                             "name": "string",
                             "value": "d"
                          }
                       ]
                    },
                    {
                       "name": "contact",
                       "value": [
                          {
                             "name": "Tand",
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
                             "name": "string",
                             "value": "c"
                          }
                       ]
                    }
                 ]
              }
           ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable with contact rule 6", "[satisfiability]")
{
    // (<=(a, c) & <=(a,b)) & (C(a * (-c), d) | C(a * (-b), c))
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
                          "value": [
                             {
                                "name": "string",
                                "value": "a"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "c"
                                }
                             }
                          ]
                       }
                    },
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
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
                             "name": "Tand",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "a"
                                },
                                {
                                   "name": "Tstar",
                                   "value": {
                                      "name": "string",
                                      "value": "c"
                                   }
                                }
                             ]
                          },
                          {
                             "name": "string",
                             "value": "d"
                          }
                       ]
                    },
                    {
                       "name": "contact",
                       "value": [
                          {
                             "name": "Tand",
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
                             "name": "string",
                             "value": "c"
                          }
                       ]
                    }
                 ]
              }
           ]
        })"_json,
        false, false, false);
}

TEST_CASE("satisfiable with contact rule 6.1", "[satisfiability]")
{
    // ~(~<=(a, c) | ~<=(a,b)) & (C(a * (-c), d) | C(a * (-b), c))
    is_satisfiable(
        R"({
            "name": "conjunction",
            "value": [
               {
                  "name": "negation",
                  "value": {
                     "name": "disjunction",
                     "value": [
                        {
                           "name": "negation",
                           "value": {
                              "name": "equal0",
                              "value": {
                                 "name": "Tand",
                                 "value": [
                                    {
                                       "name": "string",
                                       "value": "a"
                                    },
                                    {
                                       "name": "Tstar",
                                       "value": {
                                          "name": "string",
                                          "value": "c"
                                       }
                                    }
                                 ]
                              }
                           }
                        },
                        {
                           "name": "negation",
                           "value": {
                              "name": "equal0",
                              "value": {
                                 "name": "Tand",
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
                              }
                           }
                        }
                     ]
                  }
               },
               {
                  "name": "disjunction",
                  "value": [
                     {
                        "name": "contact",
                        "value": [
                           {
                              "name": "Tand",
                              "value": [
                                 {
                                    "name": "string",
                                    "value": "a"
                                 },
                                 {
                                    "name": "Tstar",
                                    "value": {
                                       "name": "string",
                                       "value": "c"
                                    }
                                 }
                              ]
                           },
                           {
                              "name": "string",
                              "value": "d"
                           }
                        ]
                     },
                     {
                        "name": "contact",
                        "value": [
                           {
                              "name": "Tand",
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
                              "name": "string",
                              "value": "c"
                           }
                        ]
                     }
                  ]
               }
            ]
        })"_json,
        false, false, false);
}

TEST_CASE("satisfiable with contact rule 7", "[satisfiability]")
{
    // (~<=(a,b) & (~<=(c,d) & C(a * -b, c * -d)))
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "negation",
                 "value": {
                    "name": "equal0",
                    "value": {
                       "name": "Tand",
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
                    }
                 }
              },
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "negation",
                       "value": {
                          "name": "equal0",
                          "value": {
                             "name": "Tand",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "c"
                                },
                                {
                                   "name": "Tstar",
                                   "value": {
                                      "name": "string",
                                      "value": "d"
                                   }
                                }
                             ]
                          }
                       }
                    },
                    {
                       "name": "contact",
                       "value": [
                          {
                             "name": "Tand",
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
                             "name": "Tand",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "c"
                                },
                                {
                                   "name": "Tstar",
                                   "value": {
                                      "name": "string",
                                      "value": "d"
                                   }
                                }
                             ]
                          }
                       ]
                    }
                 ]
              }
           ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable with contact rule 8", "[satisfiability]")
{
    // (~<=(a,b) & (~<=(c,d) & ~C(a * -b, c * -d)))
    is_satisfiable(
        R"({
          "name": "conjunction",
           "value": [
              {
                 "name": "negation",
                 "value": {
                    "name": "equal0",
                    "value": {
                       "name": "Tand",
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
                    }
                 }
              },
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "negation",
                       "value": {
                          "name": "equal0",
                          "value": {
                             "name": "Tand",
                             "value": [
                                {
                                   "name": "string",
                                   "value": "c"
                                },
                                {
                                   "name": "Tstar",
                                   "value": {
                                      "name": "string",
                                      "value": "d"
                                   }
                                }
                             ]
                          }
                       }
                    },
                    {
                       "name": "negation",
                       "value": {
                          "name": "contact",
                          "value": [
                             {
                                "name": "Tand",
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
                                "name": "Tand",
                                "value": [
                                   {
                                      "name": "string",
                                      "value": "c"
                                   },
                                   {
                                      "name": "Tstar",
                                      "value": {
                                         "name": "string",
                                         "value": "d"
                                      }
                                   }
                                ]
                             }
                          ]
                       }
                    }
                 ]
              }
           ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact", "[satisfiability]")
{
    // (<=(x,y) & <=(a,b)) & C(a * -b, z)
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
                          "value": [
                             {
                                "name": "string",
                                "value": "x"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "y"
                                }
                             }
                          ]
                       }
                    },
                    {
                       "name": "equal0",
                       "value": {
                          "name": "Tand",
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
                       }
                    }
                 ]
              },
              {
                 "name": "contact",
                 "value": [
                    {
                       "name": "Tand",
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
                       "name": "string",
                       "value": "z"
                    }
                 ]
              }
           ]
        })"_json,
        false, false, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact 2", "[satisfiability]")
{
    // ((~<=(x,y) & <=(z, t)) & ~<=(a,b)) & C(a * -b, z * -t)
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "conjunction",
                       "value": [
                          {
                             "name": "negation",
                             "value": {
                                "name": "equal0",
                                "value": {
                                   "name": "Tand",
                                   "value": [
                                      {
                                         "name": "string",
                                         "value": "x"
                                      },
                                      {
                                         "name": "Tstar",
                                         "value": {
                                            "name": "string",
                                            "value": "y"
                                         }
                                      }
                                   ]
                                }
                             }
                          },
                          {
                             "name": "equal0",
                             "value": {
                                "name": "Tand",
                                "value": [
                                   {
                                      "name": "string",
                                      "value": "z"
                                   },
                                   {
                                      "name": "Tstar",
                                      "value": {
                                         "name": "string",
                                         "value": "t"
                                      }
                                   }
                                ]
                             }
                          }
                       ]
                    },
                    {
                       "name": "negation",
                       "value": {
                          "name": "equal0",
                          "value": {
                             "name": "Tand",
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
                          }
                       }
                    }
                 ]
              },
              {
                 "name": "contact",
                 "value": [
                    {
                       "name": "Tand",
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
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "z"
                          },
                          {
                             "name": "Tstar",
                             "value": {
                                "name": "string",
                                "value": "t"
                             }
                          }
                       ]
                    }
                 ]
              }
           ]
        })"_json,
        false, false, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact 3", "[satisfiability]")
{
    // ((~<=(x,y) & ~<=(z, t)) & ~<=(a,b)) & C(a * -b, z * -t)
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "conjunction",
                 "value": [
                    {
                       "name": "conjunction",
                       "value": [
                          {
                             "name": "negation",
                             "value": {
                                "name": "equal0",
                                "value": {
                                   "name": "Tand",
                                   "value": [
                                      {
                                         "name": "string",
                                         "value": "x"
                                      },
                                      {
                                         "name": "Tstar",
                                         "value": {
                                            "name": "string",
                                            "value": "y"
                                         }
                                      }
                                   ]
                                }
                             }
                          },
                          {
                             "name": "negation",
                             "value": {
                                "name": "equal0",
                                "value": {
                                   "name": "Tand",
                                   "value": [
                                      {
                                         "name": "string",
                                         "value": "z"
                                      },
                                      {
                                         "name": "Tstar",
                                         "value": {
                                            "name": "string",
                                            "value": "t"
                                         }
                                      }
                                   ]
                                }
                             }
                          }
                       ]
                    },
                    {
                       "name": "negation",
                       "value": {
                          "name": "equal0",
                          "value": {
                             "name": "Tand",
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
                          }
                       }
                    }
                 ]
              },
              {
                 "name": "contact",
                 "value": [
                    {
                       "name": "Tand",
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
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "z"
                          },
                          {
                             "name": "Tstar",
                             "value": {
                                "name": "string",
                                "value": "t"
                             }
                          }
                       ]
                    }
                 ]
              }
           ]
        })"_json,
        true, true, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact 4", "[satisfiability]")
{
    // ~( ~((<=(x,y) & ~<=(z, t)) & ~<=(a,b)) | C(a * -b, z * -t) )
    is_satisfiable(
        R"({
            "name": "negation",
            "value": {
                "name": "disjunction",
                "value": [
                    {
                    "name": "negation",
                    "value": {
                        "name": "conjunction",
                        "value": [
                            {
                                "name": "conjunction",
                                "value": [
                                {
                                    "name": "equal0",
                                    "value": {
                                        "name": "Tand",
                                        "value": [
                                            {
                                            "name": "string",
                                            "value": "x"
                                            },
                                            {
                                            "name": "Tstar",
                                            "value": {
                                                "name": "string",
                                                "value": "y"
                                            }
                                            }
                                        ]
                                    }
                                },
                                {
                                    "name": "negation",
                                    "value": {
                                        "name": "equal0",
                                        "value": {
                                            "name": "Tand",
                                            "value": [
                                            {
                                                "name": "string",
                                                "value": "z"
                                            },
                                            {
                                                "name": "Tstar",
                                                "value": {
                                                    "name": "string",
                                                    "value": "t"
                                                }
                                            }
                                            ]
                                        }
                                    }
                                }
                                ]
                            },
                            {
                                "name": "negation",
                                "value": {
                                "name": "equal0",
                                "value": {
                                    "name": "Tand",
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
                                }
                                }
                            }
                        ]
                    }
                    },
                    {
                    "name": "contact",
                    "value": [
                        {
                            "name": "Tand",
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
                            "name": "Tand",
                            "value": [
                                {
                                "name": "string",
                                "value": "z"
                                },
                                {
                                "name": "Tstar",
                                "value": {
                                    "name": "string",
                                    "value": "t"
                                }
                                }
                            ]
                        }
                    ]
                    }
                ]
            }
        })"_json,
        true, true, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact using same terms 1",
          "[satisfiability]")
{
    // C(a * -d, b * -c) & ( (C(a * -d, d) & <=(a,d)) | <=(b,c))
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "contact",
                 "value": [
                    {
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "a"
                          },
                          {
                             "name": "Tstar",
                             "value": {
                                "name": "string",
                                "value": "d"
                             }
                          }
                       ]
                    },
                    {
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "b"
                          },
                          {
                             "name": "Tstar",
                             "value": {
                                "name": "string",
                                "value": "c"
                             }
                          }
                       ]
                    }
                 ]
              },
              {
                 "name": "disjunction",
                 "value": [
                    {
                       "name": "conjunction",
                       "value": [
                          {
                             "name": "contact",
                             "value": [
                                {
                                   "name": "Tand",
                                   "value": [
                                      {
                                         "name": "string",
                                         "value": "a"
                                      },
                                      {
                                         "name": "Tstar",
                                         "value": {
                                            "name": "string",
                                            "value": "d"
                                         }
                                      }
                                   ]
                                },
                                {
                                   "name": "string",
                                   "value": "d"
                                }
                             ]
                          },
                          {
                             "name": "equal0",
                             "value": {
                                "name": "Tand",
                                "value": [
                                   {
                                      "name": "string",
                                      "value": "a"
                                   },
                                   {
                                      "name": "Tstar",
                                      "value": {
                                         "name": "string",
                                         "value": "d"
                                      }
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
                                "name": "string",
                                "value": "b"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "c"
                                }
                             }
                          ]
                       }
                    }
                 ]
              }
           ]
        })"_json,
        false, false, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact using same terms 2",
          "[satisfiability]")
{
    // C(a * -d, b * -c) & ((C(a * -d, d) & <=(a,d)) | <=(b,x))
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "contact",
                 "value": [
                    {
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "a"
                          },
                          {
                             "name": "Tstar",
                             "value": {
                                "name": "string",
                                "value": "d"
                             }
                          }
                       ]
                    },
                    {
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "b"
                          },
                          {
                             "name": "Tstar",
                             "value": {
                                "name": "string",
                                "value": "c"
                             }
                          }
                       ]
                    }
                 ]
              },
              {
                 "name": "disjunction",
                 "value": [
                    {
                       "name": "conjunction",
                       "value": [
                          {
                             "name": "contact",
                             "value": [
                                {
                                   "name": "Tand",
                                   "value": [
                                      {
                                         "name": "string",
                                         "value": "a"
                                      },
                                      {
                                         "name": "Tstar",
                                         "value": {
                                            "name": "string",
                                            "value": "d"
                                         }
                                      }
                                   ]
                                },
                                {
                                   "name": "string",
                                   "value": "d"
                                }
                             ]
                          },
                          {
                             "name": "equal0",
                             "value": {
                                "name": "Tand",
                                "value": [
                                   {
                                      "name": "string",
                                      "value": "a"
                                   },
                                   {
                                      "name": "Tstar",
                                      "value": {
                                         "name": "string",
                                         "value": "d"
                                      }
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
                                "name": "string",
                                "value": "b"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "x"
                                }
                             }
                          ]
                       }
                    }
                 ]
              }
           ]
        })"_json,
        true, true, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact using same terms 2.1",
          "[satisfiability]")
{
    // C(a * -d, b * -c) & ( (C(a * -d, -d) & <=(a,d)) | <=(b,x)))
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "contact",
                 "value": [
                    {
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "a"
                          },
                          {
                             "name": "Tstar",
                             "value": {
                                "name": "string",
                                "value": "d"
                             }
                          }
                       ]
                    },
                    {
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "b"
                          },
                          {
                             "name": "Tstar",
                             "value": {
                                "name": "string",
                                "value": "c"
                             }
                          }
                       ]
                    }
                 ]
              },
              {
                 "name": "disjunction",
                 "value": [
                    {
                       "name": "conjunction",
                       "value": [
                          {
                             "name": "contact",
                             "value": [
                                {
                                   "name": "Tand",
                                   "value": [
                                      {
                                         "name": "string",
                                         "value": "a"
                                      },
                                      {
                                         "name": "Tstar",
                                         "value": {
                                            "name": "string",
                                            "value": "d"
                                         }
                                      }
                                   ]
                                },
                                {
                                    "name": "Tstar",
                                    "value": {
                                       "name": "string",
                                       "value": "d"
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
                                      "name": "string",
                                      "value": "a"
                                   },
                                   {
                                      "name": "Tstar",
                                      "value": {
                                         "name": "string",
                                         "value": "d"
                                      }
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
                                "name": "string",
                                "value": "b"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "x"
                                }
                             }
                          ]
                       }
                    }
                 ]
              }
           ]
        })"_json,
        true, true, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact using same terms 3",
          "[satisfiability]")
{
    // ~C(a * -d, b * -c) & ( (~C(a * -d, d * -c) & (~<=(a,d) & ~<=(d,c))) | (~<=(b,c) & ~<=(a,d)) )
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "negation",
                 "value": {
                    "name": "contact",
                    "value": [
                       {
                          "name": "Tand",
                          "value": [
                             {
                                "name": "string",
                                "value": "a"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "d"
                                }
                             }
                          ]
                       },
                       {
                          "name": "Tand",
                          "value": [
                             {
                                "name": "string",
                                "value": "b"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "c"
                                }
                             }
                          ]
                       }
                    ]
                 }
              },
              {
                 "name": "disjunction",
                 "value": [
                    {
                       "name": "conjunction",
                       "value": [
                          {
                             "name": "negation",
                             "value": {
                                "name": "contact",
                                "value": [
                                   {
                                      "name": "Tand",
                                      "value": [
                                         {
                                            "name": "string",
                                            "value": "a"
                                         },
                                         {
                                            "name": "Tstar",
                                            "value": {
                                               "name": "string",
                                               "value": "d"
                                            }
                                         }
                                      ]
                                   },
                                   {
                                      "name": "Tand",
                                      "value": [
                                         {
                                            "name": "string",
                                            "value": "d"
                                         },
                                         {
                                            "name": "Tstar",
                                            "value": {
                                               "name": "string",
                                               "value": "c"
                                            }
                                         }
                                      ]
                                   }
                                ]
                             }
                          },
                          {
                             "name": "conjunction",
                             "value": [
                                {
                                   "name": "negation",
                                   "value": {
                                      "name": "equal0",
                                      "value": {
                                         "name": "Tand",
                                         "value": [
                                            {
                                               "name": "string",
                                               "value": "a"
                                            },
                                            {
                                               "name": "Tstar",
                                               "value": {
                                                  "name": "string",
                                                  "value": "d"
                                               }
                                            }
                                         ]
                                      }
                                   }
                                },
                                {
                                   "name": "negation",
                                   "value": {
                                      "name": "equal0",
                                      "value": {
                                         "name": "Tand",
                                         "value": [
                                            {
                                               "name": "string",
                                               "value": "d"
                                            },
                                            {
                                               "name": "Tstar",
                                               "value": {
                                                  "name": "string",
                                                  "value": "c"
                                               }
                                            }
                                         ]
                                      }
                                   }
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
                                "name": "equal0",
                                "value": {
                                   "name": "Tand",
                                   "value": [
                                      {
                                         "name": "string",
                                         "value": "b"
                                      },
                                      {
                                         "name": "Tstar",
                                         "value": {
                                            "name": "string",
                                            "value": "c"
                                         }
                                      }
                                   ]
                                }
                             }
                          },
                          {
                             "name": "negation",
                             "value": {
                                "name": "equal0",
                                "value": {
                                   "name": "Tand",
                                   "value": [
                                      {
                                         "name": "string",
                                         "value": "a"
                                      },
                                      {
                                         "name": "Tstar",
                                         "value": {
                                            "name": "string",
                                            "value": "d"
                                         }
                                      }
                                   ]
                                }
                             }
                          }
                       ]
                    }
                 ]
              }
           ]
        })"_json,
        true, true, false);
}

TEST_CASE("satisfiable with contact rule - adding non zero term and F contact with same left/right child 1",
          "[satisfiability]")
{
    // ~C(a * -d, a * -d) & ~<=(a,d)
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "negation",
                 "value": {
                    "name": "contact",
                    "value": [
                       {
                          "name": "Tand",
                          "value": [
                             {
                                "name": "string",
                                "value": "a"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "d"
                                }
                             }
                          ]
                       },
                       {
                          "name": "Tand",
                          "value": [
                             {
                                "name": "string",
                                "value": "a"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "d"
                                }
                             }
                          ]
                       }
                    ]
                 }
              },
              {
                 "name": "negation",
                 "value": {
                    "name": "equal0",
                    "value": {
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "a"
                          },
                          {
                             "name": "Tstar",
                             "value": {
                                "name": "string",
                                "value": "d"
                             }
                          }
                       ]
                    }
                 }
              }
           ]
        })"_json,
        false, false);
}

TEST_CASE("satisfiable with contact rule - adding non zero term and F contact with same left/right child 2",
          "[satisfiability]")
{
    // ~<=(a,d) & ~C(a * -d, a * -d)
    is_satisfiable(
        R"({
           "name": "conjunction",
           "value": [
              {
                 "name": "negation",
                 "value": {
                    "name": "equal0",
                    "value": {
                       "name": "Tand",
                       "value": [
                          {
                             "name": "string",
                             "value": "a"
                          },
                          {
                             "name": "Tstar",
                             "value": {
                                "name": "string",
                                "value": "d"
                             }
                          }
                       ]
                    }
                 }
              },
              {
                 "name": "negation",
                 "value": {
                    "name": "contact",
                    "value": [
                       {
                          "name": "Tand",
                          "value": [
                             {
                                "name": "string",
                                "value": "a"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "d"
                                }
                             }
                          ]
                       },
                       {
                          "name": "Tand",
                          "value": [
                             {
                                "name": "string",
                                "value": "a"
                             },
                             {
                                "name": "Tstar",
                                "value": {
                                   "name": "string",
                                   "value": "d"
                                }
                             }
                          ]
                       }
                    ]
                 }
              }
           ]
        })"_json,
        false, false);
}

TEST_CASE("satisfiable evaluation of atomic formulas when there is a constant 1", "[satisfiability]")
{
    // C(a, -a) & ~(F)
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
                           "value": "a"
                        }
                     }
                  ]
               },
               {
                  "name": "negation",
                  "value": {
                     "name": "F"
                  }
               }
            ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable evaluation of atomic formulas when there is a constant 2", "[satisfiability]")
{
    // C(a, -a) & ~(C(e,f) & ~T)
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
                           "value": "a"
                        }
                     }
                  ]
               },
               {
                  "name": "negation",
                  "value": {
                     "name": "conjunction",
                     "value": [
                        {
                           "name": "contact",
                           "value": [
                              {
                                 "name": "string",
                                 "value": "e"
                              },
                              {
                                 "name": "string",
                                 "value": "f"
                              }
                           ]
                        },
                        {
                           "name": "negation",
                           "value": {
                              "name": "T"
                           }
                        }
                     ]
                  }
               }
            ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable evaluation of atomic formulas when there is a constant 3", "[satisfiability]")
{
    // C(a, -a) & (C(f,g) | T)
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
                           "value": "a"
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
                              "value": "f"
                           },
                           {
                              "name": "string",
                              "value": "g"
                           }
                        ]
                     },
                     {
                        "name": "T"
                     }
                  ]
               }
            ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable evaluation of atomic formulas when there is a constant 4", "[satisfiability]")
{
    // C(a, -a) & ~(C(f,g) & F)
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
                           "value": "a"
                        }
                     }
                  ]
               },
               {
                  "name": "negation",
                  "value": {
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
                                 "value": "g"
                              }
                           ]
                        },
                        {
                           "name": "F"
                        }
                     ]
                  }
               }
            ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable when F disjunction has contradicting right child 1", "[satisfiability]")
{
    // ~(C(a,b) | T) | C(a,b)
    is_satisfiable(
        R"({
            "name": "disjunction",
            "value": [
               {
                  "name": "negation",
                  "value": {
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
                           "name": "T"
                        }
                     ]
                  }
               },
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
               }
            ]
        })"_json,
        true, true);
}

TEST_CASE("satisfiable when T conjunction has contradicting right child 1", "[satisfiability]")
{
    // (C(a,b) & F) | ~C(a,b)
    is_satisfiable(
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
                        "name": "F"
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
        true, true);
}
