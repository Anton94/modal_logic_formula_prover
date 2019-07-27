#include "catch/catch.hpp"

#include "library.h"
#include "nlohmann_json/json.hpp"
#include "term.h"

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

void check_term_varibles_mask(const term& t, const formula_mgr& mgr, const variables_set_t& expected_variables)
{
    info() << "Checking: " << t << " to have the following variables: " << expected_variables << "in it's bitset mask. "
        << "Mgr has the following variables: " << mgr.get_variables();

    const auto& term_vars_mask = t.get_variables();
    CHECK(term_vars_mask.count() == expected_variables.size());

    for (size_t i = 0; i < term_vars_mask.size(); ++i)
    {
        const auto var_name = mgr.get_variable(i);
        if (term_vars_mask[i])
        {
            CHECK(expected_variables.find(var_name) != expected_variables.end());
        }
        else
        {
            CHECK(expected_variables.find(var_name) == expected_variables.end());
        }
    }
}

}

TEST_CASE("complex term's variables", "[variables_check_term]")
{
    // ((-a + b) * (c + -d)) + -(e * (f + g))
    json term_json =
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
    auto formula_mgr = create_atomic_formula_with_same_child_terms(term_json);
    auto term = formula_mgr.get_internal_formula()->get_left_child_term();

    check_term_varibles_mask(*term, formula_mgr, { "a", "b", "c", "d", "e", "f", "g" });

    CHECK(term->get_operation_type() == term::operation_t::union_);
    auto t_l = term->get_left_child(); // (-a + b) * (c + -d)
    auto t_r = term->get_right_child(); // -(e * (f + g))
    check_term_varibles_mask(*t_l, formula_mgr, { "a", "b", "c", "d" });
    check_term_varibles_mask(*t_r, formula_mgr, { "e", "f", "g" });

    // (-a + b) * (c + -d)
    CHECK(t_l->get_operation_type() == term::operation_t::intersaction_);
    auto t_l_l = t_l->get_left_child(); // -a + b
    auto t_l_r = t_l->get_right_child(); // c + -d
    check_term_varibles_mask(*t_l_l, formula_mgr, { "a", "b"});
    check_term_varibles_mask(*t_l_r, formula_mgr, { "c", "d" });

    // -a + b
    CHECK(t_l_l->get_operation_type() == term::operation_t::union_);
    auto t_l_l_l = t_l_l->get_left_child(); // -a
    auto t_l_l_r = t_l_l->get_right_child(); // b
    check_term_varibles_mask(*t_l_l_l, formula_mgr, { "a" });
    CHECK(t_l_l_r->get_operation_type() == term::operation_t::variable_);
    check_term_varibles_mask(*t_l_l_r, formula_mgr, { "b" });

    CHECK(t_l_l_l->get_operation_type() == term::operation_t::star_);
    auto t_l_l_l_l = t_l_l_l->get_left_child(); // a
    CHECK(t_l_l_l_l->get_operation_type() == term::operation_t::variable_);
    check_term_varibles_mask(*t_l_l_l_l, formula_mgr, { "a" });

    // c + -d
    CHECK(t_l_r->get_operation_type() == term::operation_t::union_);
    auto t_l_r_l = t_l_r->get_left_child(); // c
    auto t_l_r_r = t_l_r->get_right_child(); // -d
    CHECK(t_l_r_l->get_operation_type() == term::operation_t::variable_);
    check_term_varibles_mask(*t_l_r_l, formula_mgr, { "c" });
    check_term_varibles_mask(*t_l_r_r, formula_mgr, { "d" });

    CHECK(t_l_r_r->get_operation_type() == term::operation_t::star_);
    auto t_l_r_r_l = t_l_r_r->get_left_child(); // d
    CHECK(t_l_r_r_l->get_operation_type() == term::operation_t::variable_);
    check_term_varibles_mask(*t_l_r_r_l, formula_mgr, { "d" });

    // Done with (-a + b) * (c + -d)
    // -(e * (f + g))
    CHECK(t_r->get_operation_type() == term::operation_t::star_);
    auto t_r_l = t_r->get_left_child(); // e * (f + g)
    check_term_varibles_mask(*t_r_l, formula_mgr, { "e", "f", "g" });

    CHECK(t_r_l->get_operation_type() == term::operation_t::intersaction_);
    auto t_r_l_l = t_r_l->get_left_child(); // e
    auto t_r_l_r = t_r_l->get_right_child(); // f + g
    CHECK(t_r_l_l->get_operation_type() == term::operation_t::variable_);
    check_term_varibles_mask(*t_r_l_l, formula_mgr, { "e" });
    check_term_varibles_mask(*t_r_l_r, formula_mgr, { "f", "g" });

    // f + g
    CHECK(t_r_l_r->get_operation_type() == term::operation_t::union_);
    auto t_r_l_r_l = t_r_l_r->get_left_child(); // f
    auto t_r_l_r_r = t_r_l_r->get_right_child(); // g
    CHECK(t_r_l_r_l->get_operation_type() == term::operation_t::variable_);
    check_term_varibles_mask(*t_r_l_r_l, formula_mgr, { "f" });
    CHECK(t_r_l_r_r->get_operation_type() == term::operation_t::variable_);
    check_term_varibles_mask(*t_r_l_r_r, formula_mgr, { "g"});
}

TEST_CASE("variables of term containing a constant", "[variables_check_term]")
{
    // 0
    json term_json =
        R"({
         "name": "0"
      })"_json;
    auto formula_mgr = create_atomic_formula_with_same_child_terms(term_json);
    auto term = formula_mgr.get_internal_formula()->get_left_child_term();

    CHECK(term->get_operation_type() == term::operation_t::constant_false);
    check_term_varibles_mask(*term, formula_mgr, { });
}

TEST_CASE("variables of term containing a variable", "[variables_check_term]")
{
    // a
    json term_json =
        R"({
         "name": "string",
         "value": "a"
      })"_json;
    auto formula_mgr = create_atomic_formula_with_same_child_terms(term_json);
    auto term = formula_mgr.get_internal_formula()->get_left_child_term();

    CHECK(term->get_operation_type() == term::operation_t::variable_);
    check_term_varibles_mask(*term, formula_mgr, { "a" });
}

TEST_CASE("variables of term containing unary star operation of variable", "[variables_check_term]")
{
    // -a
    json term_json =
        R"({
         "name": "Tstar",
         "value": {
            "name": "string",
            "value": "a"
         }
      })"_json;
    auto formula_mgr = create_atomic_formula_with_same_child_terms(term_json);
    auto term = formula_mgr.get_internal_formula()->get_left_child_term();

    CHECK(term->get_operation_type() == term::operation_t::star_);
    check_term_varibles_mask(*term, formula_mgr, { "a" });

    auto t_l = term->get_left_child(); // a
    CHECK(t_l->get_operation_type() == term::operation_t::variable_);
    check_term_varibles_mask(*t_l, formula_mgr, { "a" });
}
