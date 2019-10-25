#include "catch/catch.hpp"

#include "library.h"
#include "term.h"

namespace
{

formula_mgr build(const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b)
{
    // the two formulas should be subformulas in a bigger one in order to the variables
    // be dependent on the number of variables used in the whole formula
    // otherwise C(a, X) will be equal to C(b, X) because 'a' and 'b' are converted to id's
    // which will be in both cases 0

    std::stringstream f_str;
    f_str << "C(" << a.first << "," << a.second << ") | C(" << b.first << "," << b.second << ")";

    formula_mgr f;
    CHECK(f.build(f_str.str(), formula_mgr::formula_refiners::none));

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

void check_equality_of_contacts_with_terms(const std::string& a, const std::string& b)
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

// TODO: fix the tests, the parser is making a lot of optimizations and the majority of the assert's are not OK to be tests as is
// Maybe if we disable the converters/reducers after the formula is passed it will be OK, or we can think of another test suits.
TEST_CASE("complex terms 0", "[contact_commutativity]")
{
    check_equality_of_contacts_with_terms("((-a + b) * (c + -d)) + -(e * (f + g))", "((-a + b) * (c + -d)) + -(m * (f + g))" /* Note the 'm' variable */);
}

TEST_CASE("complex terms 1", "[contact_commutativity]")
{
    check_equality_of_contacts_with_terms("((-a + b) * (c + -d)) + -(e * (f + g))", "((-a + b) * (c + -d)) + -(e * f)");
}

TEST_CASE("constants", "[contact_commutativity]")
{
    check_equality_of_contacts_with_terms("0", "1");
}

TEST_CASE("variables", "[contact_commutativity]")
{
    check_equality_of_contacts_with_terms("a", "b");
}

TEST_CASE("variable and negation of variable", "[contact_commutativity]")
{
    check_equality_of_contacts_with_terms("a", "-b");
}

TEST_CASE("negation of variables", "[contact_commutativity]")
{
    check_equality_of_contacts_with_terms("-a", "-b");
}

TEST_CASE("negation of variable and negation of constant", "[contact_commutativity]")
{
    check_equality_of_contacts_with_terms("-a", "-0");
}
