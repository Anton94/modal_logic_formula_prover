#include "catch/catch.hpp"

#include "library.h"
#include "term.h"

namespace
{

formula_mgr create_atomic_formula_with_same_child_terms(const std::string& term)
{
    std::string C_t;
    C_t.append("C(");
    C_t.append(term);
    C_t.append(",");
    C_t.append(term);
    C_t.append(")");

    formula_mgr f;
    CHECK(f.build(C_t, formula_mgr::formula_refiners::none));

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
    auto formula_mgr = create_atomic_formula_with_same_child_terms("((-a + b) * (c + -d)) + -(e * (f + g))");
    auto term = formula_mgr.get_internal_formula()->get_left_child_term();

    check_term_varibles_mask(*term, formula_mgr, { "a", "b", "c", "d", "e", "f", "g" });

    CHECK(term->get_operation_type() == term::operation_t::union_);
    auto t_l = term->get_left_child(); // (-a + b) * (c + -d)
    auto t_r = term->get_right_child(); // -(e * (f + g))
    check_term_varibles_mask(*t_l, formula_mgr, { "a", "b", "c", "d" });
    check_term_varibles_mask(*t_r, formula_mgr, { "e", "f", "g" });

    // (-a + b) * (c + -d)
    CHECK(t_l->get_operation_type() == term::operation_t::intersection);
    auto t_l_l = t_l->get_left_child(); // -a + b
    auto t_l_r = t_l->get_right_child(); // c + -d
    check_term_varibles_mask(*t_l_l, formula_mgr, { "a", "b"});
    check_term_varibles_mask(*t_l_r, formula_mgr, { "c", "d" });

    // -a + b
    CHECK(t_l_l->get_operation_type() == term::operation_t::union_);
    auto t_l_l_l = t_l_l->get_left_child(); // -a
    auto t_l_l_r = t_l_l->get_right_child(); // b
    check_term_varibles_mask(*t_l_l_l, formula_mgr, { "a" });
    CHECK(t_l_l_r->get_operation_type() == term::operation_t::variable);
    check_term_varibles_mask(*t_l_l_r, formula_mgr, { "b" });

    CHECK(t_l_l_l->get_operation_type() == term::operation_t::complement);
    auto t_l_l_l_l = t_l_l_l->get_left_child(); // a
    CHECK(t_l_l_l_l->get_operation_type() == term::operation_t::variable);
    check_term_varibles_mask(*t_l_l_l_l, formula_mgr, { "a" });

    // c + -d
    CHECK(t_l_r->get_operation_type() == term::operation_t::union_);
    auto t_l_r_l = t_l_r->get_left_child(); // c
    auto t_l_r_r = t_l_r->get_right_child(); // -d
    CHECK(t_l_r_l->get_operation_type() == term::operation_t::variable);
    check_term_varibles_mask(*t_l_r_l, formula_mgr, { "c" });
    check_term_varibles_mask(*t_l_r_r, formula_mgr, { "d" });

    CHECK(t_l_r_r->get_operation_type() == term::operation_t::complement);
    auto t_l_r_r_l = t_l_r_r->get_left_child(); // d
    CHECK(t_l_r_r_l->get_operation_type() == term::operation_t::variable);
    check_term_varibles_mask(*t_l_r_r_l, formula_mgr, { "d" });

    // Done with (-a + b) * (c + -d)
    // -(e * (f + g))
    CHECK(t_r->get_operation_type() == term::operation_t::complement);
    auto t_r_l = t_r->get_left_child(); // e * (f + g)
    check_term_varibles_mask(*t_r_l, formula_mgr, { "e", "f", "g" });

    CHECK(t_r_l->get_operation_type() == term::operation_t::intersection);
    auto t_r_l_l = t_r_l->get_left_child(); // e
    auto t_r_l_r = t_r_l->get_right_child(); // f + g
    CHECK(t_r_l_l->get_operation_type() == term::operation_t::variable);
    check_term_varibles_mask(*t_r_l_l, formula_mgr, { "e" });
    check_term_varibles_mask(*t_r_l_r, formula_mgr, { "f", "g" });

    // f + g
    CHECK(t_r_l_r->get_operation_type() == term::operation_t::union_);
    auto t_r_l_r_l = t_r_l_r->get_left_child(); // f
    auto t_r_l_r_r = t_r_l_r->get_right_child(); // g
    CHECK(t_r_l_r_l->get_operation_type() == term::operation_t::variable);
    check_term_varibles_mask(*t_r_l_r_l, formula_mgr, { "f" });
    CHECK(t_r_l_r_r->get_operation_type() == term::operation_t::variable);
    check_term_varibles_mask(*t_r_l_r_r, formula_mgr, { "g"});
}

TEST_CASE("variables of term containing a constant", "[variables_check_term]")
{
    auto formula_mgr = create_atomic_formula_with_same_child_terms("0");
    auto term = formula_mgr.get_internal_formula()->get_left_child_term();

    CHECK(term->get_operation_type() == term::operation_t::constant_false);
    check_term_varibles_mask(*term, formula_mgr, { });
}

TEST_CASE("variables of term containing a variable", "[variables_check_term]")
{
    auto formula_mgr = create_atomic_formula_with_same_child_terms("a");
    auto term = formula_mgr.get_internal_formula()->get_left_child_term();

    CHECK(term->get_operation_type() == term::operation_t::variable);
    check_term_varibles_mask(*term, formula_mgr, { "a" });
}

TEST_CASE("variables of term containing unary star operation of variable", "[variables_check_term]")
{
    auto formula_mgr = create_atomic_formula_with_same_child_terms("-a");
    auto term = formula_mgr.get_internal_formula()->get_left_child_term();

    CHECK(term->get_operation_type() == term::operation_t::complement);
    check_term_varibles_mask(*term, formula_mgr, { "a" });

    auto t_l = term->get_left_child(); // a
    CHECK(t_l->get_operation_type() == term::operation_t::variable);
    check_term_varibles_mask(*t_l, formula_mgr, { "a" });
}
