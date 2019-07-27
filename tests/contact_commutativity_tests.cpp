#include "catch/catch.hpp"

#include "library.h"
#include "nlohmann_json/json.hpp"
#include "term.h"

using json = nlohmann::json;

namespace
{

formula_mgr build(const std::pair<json, json>& a, const std::pair<json, json>& b)
{
    // the two formulas should be subformulas in a bigger one in order to the variables
    // be dependent on the number of variables used in the whole formula
    // otherwise C(a, X) will be equal to C(b, X) because 'a' and 'b' are converted to id's
    // which will be in both cases 0

    // create C(a.first, a.second)
    json contact_a;
    contact_a["name"] = "contact";
    json contact_a_term;
    contact_a_term[0] = a.first;
    contact_a_term[1] = a.second;
    contact_a["value"] = contact_a_term;

    // create C(b.first, b.second)
    json contact_b;
    contact_b["name"] = "contact";
    json contact_b_term;
    contact_b_term[0] = b.first;
    contact_b_term[1] = b.second;
    contact_b["value"] = contact_b_term;

    json formula;
    formula["name"] = "disjunction"; // any binary formula operation is sufficient
    formula["value"][0] = contact_a;
    formula["value"][1] = contact_b;

    formula_mgr f;
    CHECK(f.build(formula));

    return f;
}

void check_eq(const formula_mgr& mgr, bool expected)
{
    const auto h = mgr.get_internal_formula();
    assert(h->get_operation_type() == formula::operation_t::disjunction);

    const auto f = h->get_left_child_formula();
    const auto g = h->get_right_child_formula();
    assert(f->get_operation_type() == formula::operation_t::c);
    assert(g->get_operation_type() == formula::operation_t::c);

    if(expected)
    {
        CHECK(*f == *g);
        // note that the equality operators check also the hash of the the formulas, but in any case...
        CHECK(f->get_hash() == g->get_hash());
    }
    else
    {
        CHECK(*f != *g);
        CHECK(f->get_hash() != g->get_hash());
    }
}

void check_equality_of_contacts_with_terms(const json& a, const json& b)
{
    assert(a != b);

    check_eq(build({a, a}, {a, a}), true);

    check_eq(build({b, b}, {b, b}), true);

    check_eq(build({a, b}, {a, b}), true);

    check_eq(build({a, b}, {b, a}), true);

    check_eq(build({b, a}, {a, b}), true);

    check_eq(build({b, a}, {b, a}), true);

    check_eq(build({a, b}, {a, a}), false);

    check_eq(build({a, b}, {b, b}), false);

    check_eq(build({b, a}, {a, a}), false);

    check_eq(build({b, a}, {b, b}), false);

    check_eq(build({a, a}, {a, b}), false);

    check_eq(build({a, a}, {b, a}), false);

    check_eq(build({b, b}, {a, b}), false);

    check_eq(build({b, b}, {b, a}), false);

    check_eq(build({b, b}, {a, a}), false);

    check_eq(build({a, a}, {b, b}), false);
}
}

TEST_CASE("complex terms 0", "[contact_commutativity]")
{
    // ((-a + b) * (c + -d)) + -(e * (f + g))
    json a =
        R"({
            "name": "Tor",
            "value": [
               {
                  "name": "Tand",
                  "value": [
                     {
                        "name": "Tor",
                        "value": [
                           {
                              "name": "Tstar",
                              "value": {
                                 "name": "string",
                                 "value": "a"
                              }
                           },
                           {
                              "name": "string",
                              "value": "b"
                           }
                        ]
                     },
                     {
                        "name": "Tor",
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
               },
               {
                  "name": "Tstar",
                  "value": {
                     "name": "Tand",
                     "value": [
                        {
                           "name": "string",
                           "value": "e"
                        },
                        {
                           "name": "Tor",
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
                        }
                     ]
                  }
               }
            ]
        })"_json;

    // ((-a + b) * (c + -d)) + -(m * (f + g)) Note the 'm' variable
    json b =
        R"({
            "name": "Tor",
            "value": [
               {
                  "name": "Tand",
                  "value": [
                     {
                        "name": "Tor",
                        "value": [
                           {
                              "name": "Tstar",
                              "value": {
                                 "name": "string",
                                 "value": "a"
                              }
                           },
                           {
                              "name": "string",
                              "value": "b"
                           }
                        ]
                     },
                     {
                        "name": "Tor",
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
               },
               {
                  "name": "Tstar",
                  "value": {
                     "name": "Tand",
                     "value": [
                        {
                           "name": "string",
                           "value": "m"
                        },
                        {
                           "name": "Tor",
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
                        }
                     ]
                  }
               }
            ]
        })"_json;
    check_equality_of_contacts_with_terms(a, b);
}

TEST_CASE("complex terms 1", "[contact_commutativity]")
{
    // ((-a + b) * (c + -d)) + -(e * (f + g))
    json a =
        R"({
            "name": "Tor",
            "value": [
               {
                  "name": "Tand",
                  "value": [
                     {
                        "name": "Tor",
                        "value": [
                           {
                              "name": "Tstar",
                              "value": {
                                 "name": "string",
                                 "value": "a"
                              }
                           },
                           {
                              "name": "string",
                              "value": "b"
                           }
                        ]
                     },
                     {
                        "name": "Tor",
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
               },
               {
                  "name": "Tstar",
                  "value": {
                     "name": "Tand",
                     "value": [
                        {
                           "name": "string",
                           "value": "e"
                        },
                        {
                           "name": "Tor",
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
                        }
                     ]
                  }
               }
            ]
        })"_json;

    // ((-a + b) * (c + -d)) + -(e * f)
    json b =
        R"({
            "name": "Tor",
            "value": [
               {
                  "name": "Tand",
                  "value": [
                     {
                        "name": "Tor",
                        "value": [
                           {
                              "name": "Tstar",
                              "value": {
                                 "name": "string",
                                 "value": "a"
                              }
                           },
                           {
                              "name": "string",
                              "value": "b"
                           }
                        ]
                     },
                     {
                        "name": "Tor",
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
               },
               {
                  "name": "Tstar",
                  "value": {
                     "name": "Tand",
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
                  }
               }
            ]
        })"_json;
    check_equality_of_contacts_with_terms(a, b);
}

TEST_CASE("constants", "[contact_commutativity]")
{
    // 0
    json a =
        R"({
         "name": "0"
      })"_json;
    // 1
    json b =
        R"({
         "name": "1"
      })"_json;
    check_equality_of_contacts_with_terms(a, b);
}

TEST_CASE("variables", "[contact_commutativity]")
{
    // a
    json a =
        R"({
         "name": "string",
         "value": "a"
      })"_json;
    // b
    json b =
        R"({
         "name": "string",
         "value": "b"
      })"_json;
    check_equality_of_contacts_with_terms(a, b);
}

TEST_CASE("variable and negation of variable", "[contact_commutativity]")
{
    // a
    json a =
        R"({
             "name": "string",
             "value": "a"
          })"_json;
    // -b
    json b =
        R"({
         "name": "Tstar",
         "value": {
            "name": "string",
            "value": "b"
         }
      })"_json;

    check_equality_of_contacts_with_terms(a, b);
}

TEST_CASE("negation of variables", "[contact_commutativity]")
{
    // -a
    json a =
        R"({
         "name": "Tstar",
         "value": {
            "name": "string",
            "value": "a"
         }
      })"_json;
    // -b
    json b =
        R"({
         "name": "Tstar",
         "value": {
            "name": "string",
            "value": "b"
         }
      })"_json;

    check_equality_of_contacts_with_terms(a, b);
}

TEST_CASE("negation of variable and negation of constant", "[contact_commutativity]")
{
    // -a
    json a =
        R"({
         "name": "Tstar",
         "value": {
            "name": "string",
            "value": "a"
         }
      })"_json;
    // -0
    json b =
        R"({
         "name": "Tstar",
         "value": {
            "name": "0"
         }
      })"_json;

    check_equality_of_contacts_with_terms(a, b);
}
