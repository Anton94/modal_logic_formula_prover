#include "catch/catch.hpp"

#include "library.h"
#include "nlohmann_json/json.hpp"
#include "term.h"
#include "variables_evaluations_block.h"

#include <utility>
#include <vector>
using json = nlohmann::json;

namespace
{

formula_mgr create_atomic_formula_with_same_child_terms(const json& term)
{
    json formula;
    formula["name"] = "contact";
    json terms;
    terms[0] = term;
    terms[1] = term;
    formula["value"] = terms;

    formula_mgr f;
    CHECK(f.build(formula));

    return f;
}

formula_mgr create_atomic_formula(const json& left, const json& right)
{
    json formula;
    formula["name"] = "contact";
    json terms;
    terms[0] = left;
    terms[1] = right;
    formula["value"] = terms;

    formula_mgr f;
    CHECK(f.build(formula));

    return f;
}

void evaluate(const json& term_for_evaluation, const json& expected_term,
              const std::vector<std::pair<std::string, bool>>& evaluations)
{
    auto mgr = create_atomic_formula_with_same_child_terms(term_for_evaluation);
    auto t = mgr.get_internal_formula()->get_left_child_term();

    variables_mask_t variables_mask(t->get_variables().size(), false);
    variables_evaluations_t evaluation_mask(t->get_variables().size(), false);
    for(const auto& evaluation : evaluations)
    {
        const auto& variable_name = evaluation.first;
        const auto value = evaluation.second;
        const auto varialbe_id = mgr.get_variable(variable_name);
        variables_mask.set(varialbe_id);
        evaluation_mask.set(varialbe_id, value);
    }

    variables_evaluations_block block(variables_mask);
    block.get_evaluations() = evaluation_mask;

    trace() << "Evaluating " << *t << " with: ";
    for(size_t i = 0, size = variables_mask.size(); i < size; ++i)
    {
        if(variables_mask[i])
        {
            trace() << "    " << mgr.get_variable(i) << " : " << evaluation_mask[i];
        }
    }

    auto res = t->evaluate(block);
    term* evaluated_term{nullptr};
    if(res.type == term::evaluation_result::result_type::term)
    {
        evaluated_term = res.release();
        trace() << "Result: " << *evaluated_term;
    }
    else
    {
        trace() << "Result: " << (res.is_constant_true() ? "constant true" : "constant false");
    }

    if(expected_term.is_boolean())
    {
        if(expected_term.get<bool>())
        {
            CHECK(res.is_constant_true());
        }
        else
        {
            CHECK(res.is_constant_false());
        }
    }
    else
    {
        auto mgr_expected = create_atomic_formula(term_for_evaluation, expected_term);
        auto expected = mgr_expected.get_internal_formula()->get_right_child_term();
        CHECK(*evaluated_term == *expected);
    }
    delete evaluated_term;
}
}

TEST_CASE("1", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + c
        R"({
            "name": "Tor",
            "value": [
               {
                  "name": "Tand",
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
                  "name": "string",
                  "value": "c"
               }
            ]
        })"_json,
        true, {{"a", true}, {"b", false}, {"c", true}});
}

TEST_CASE("2", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + c
        R"({
            "name": "Tor",
            "value": [
               {
                  "name": "Tand",
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
                  "name": "string",
                  "value": "c"
               }
            ]
        })"_json,
        true, {{"a", true}, {"b", true}, {"c", false}});
}

TEST_CASE("3", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + c
        R"({
            "name": "Tor",
            "value": [
               {
                  "name": "Tand",
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
                  "name": "string",
                  "value": "c"
               }
            ]
        })"_json,
        false, {{"a", true}, {"b", false}, {"c", false}});
}

TEST_CASE("4", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        true, {{"a", true}, {"b", false}, {"c", false}});
}

TEST_CASE("5", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        true, {{"a", true}, {"b", true}, {"c", false}});
}

TEST_CASE("6", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        false, {{"a", false}, {"b", true}, {"c", false}});
}

TEST_CASE("7", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // -c
        R"({
            "name": "Tstar",
            "value": {
               "name": "string",
               "value": "c"
            }
        })"_json,
        {{"a", false}, {"b", false}});
}

TEST_CASE("8", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // -c
        R"({
            "name": "Tstar",
            "value": {
               "name": "string",
               "value": "c"
            }
        })"_json,
        {{"a", true}, {"b", false}});
}

TEST_CASE("9", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // a
        R"({
            "name": "string",
            "value": "a"
        })"_json,
        {{"b", true}, {"c", true}});
}

TEST_CASE("10", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // a
        R"({
            "name": "string",
            "value": "a"
        })"_json,
        {{"b", true}});
}

TEST_CASE("11", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // a * b
        R"({
            "name": "Tand",
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
        {{"c", true}});
}

TEST_CASE("12", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // b + -b
        R"({
            "name": "Tor",
            "value": [
               {
                  "name": "string",
                  "value": "b"
               },
               {
                  "name": "Tstar",
                  "value": {
                     "name": "string",
                     "value": "b"
                  }
               }
            ]
        })"_json,
        {{"c", false}, {"a", true}});
}

TEST_CASE("13", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // -(b + c)
        R"({
            "name": "Tstar",
            "value": {
               "name": "Tor",
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
        })"_json,
        {{"a", false}});
}

TEST_CASE("14", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // b + -(b + c)
        R"({
          "name": "Tor",
            "value": [
               {
                  "name": "string",
                  "value": "b"
               },
               {
                  "name": "Tstar",
                  "value": {
                     "name": "Tor",
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
        {{"a", true}});
}

TEST_CASE("15", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // -c
        R"({
            "name": "Tstar",
            "value": {
               "name": "string",
               "value": "c"
            }
        })"_json,
        {{"b", false}});
}

TEST_CASE("16", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        false,
        {{"a", false}, {"b", true}});
}

TEST_CASE("17", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        true,
        {{"a", false}, {"b", false}, {"c", false}});
}

TEST_CASE("18", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        true,
        {{"a", true}, {"b", true}, {"c", true}});
}

TEST_CASE("19", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        false,
        {{"a", true}, {"b", false}, {"c", true}});
}

TEST_CASE("20", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        false,
        {{"a", false}, {"b", true}, {"c", true}});
}

TEST_CASE("21", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        false,
        {{"a", false}, {"b", false}, {"c", true}});
}

TEST_CASE("22", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        true,
        {{"b", false}, {"c", false}});
}

TEST_CASE("23", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        false,
        {{"b", false}, {"c", true}});
}

TEST_CASE("24", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        R"({
            "name": "string",
            "value": "a"
        })"_json,
        {{"b", true}, {"c", false}});
}

TEST_CASE("25", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        R"({
            "name": "string",
            "value": "a"
        })"_json,
        {{"b", true}, {"c", true}});
}

TEST_CASE("26", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // -b
        R"({
            "name": "Tstar",
            "value": {
               "name": "string",
               "value": "b"
            }
        })"_json,
        {{"a", false}, {"c", false}});
}

TEST_CASE("27", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        false,
        {{"a", false}, {"c", true}});
}

TEST_CASE("28", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // b
        R"({
            "name": "string",
            "value": "b"
        })"_json,
        {{"a", true}, {"c", true}});
}

TEST_CASE("29", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // (a * b) + -b
        R"({
            "name": "Tor",
            "value": [
               {
                  "name": "Tand",
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
                  "name": "Tstar",
                  "value": {
                        "name": "string",
                        "value": "b"
                  }
               }
            ]
        })"_json,
        {{"c", false}});
}

TEST_CASE("without evaluation", "[term_evaluation_with_subset_variables]")
{
    evaluate(
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        // (a * b) + -(b + c)
        R"({
           "name": "Tor",
           "value": [
              {
                 "name": "Tand",
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
                 "name": "Tstar",
                 "value": {
                    "name": "Tor",
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
        {});
}
