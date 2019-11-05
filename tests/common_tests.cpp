#include "catch/catch.hpp"

#include "library.h"

TEST_CASE("move assignment of formula_mgr", "[run_with_sanitizer]")
{
    formula_mgr mgr;
    mgr.build("(C(a, b) & ~C(c, 0)) & <=(1, 0)");

    formula_mgr mgr2;
    mgr2.build("C(a, b)");

    mgr2 = std::move(mgr);

    formula_mgr mgr3 = std::move(mgr2);
}

TEST_CASE("formula_refiners", "[common]")
{
    using flags_t = formula_mgr::formula_refiners;

    flags_t flags = flags_t::reduce_constants;
    CHECK(flags == flags_t::reduce_constants);

    flags = flags_t::remove_double_negation | flags_t::convert_disjunction_in_contact_less_eq;
    CHECK(flags != flags_t::all);
    CHECK(flags != flags_t::none);
    CHECK(has_flag(flags, flags_t::remove_double_negation));
    CHECK(has_flag(flags, flags_t::convert_disjunction_in_contact_less_eq));
    CHECK(!has_flag(flags, flags_t::convert_contact_less_eq_with_same_terms));
    CHECK(!has_flag(flags, flags_t::reduce_constants));
    CHECK(!has_flag(flags, flags_t::reduce_contacts_with_constants));

    flags |= flags_t::remove_double_negation | flags_t::convert_disjunction_in_contact_less_eq;
    CHECK(flags != flags_t::all);
    CHECK(flags != flags_t::none);
    CHECK(has_flag(flags, flags_t::remove_double_negation));
    CHECK(has_flag(flags, flags_t::convert_disjunction_in_contact_less_eq));
    CHECK(!has_flag(flags, flags_t::convert_contact_less_eq_with_same_terms));
    CHECK(!has_flag(flags, flags_t::reduce_constants));
    CHECK(!has_flag(flags, flags_t::reduce_contacts_with_constants));

    flags = flags_t::none;
    CHECK(flags == flags_t::none);

    flags = flags_t::remove_double_negation | flags_t::convert_disjunction_in_contact_less_eq | flags_t::convert_contact_less_eq_with_same_terms;
    CHECK(flags != flags_t::all);
    CHECK(flags != flags_t::none);
    CHECK(has_flag(flags, flags_t::remove_double_negation));
    CHECK(has_flag(flags, flags_t::convert_disjunction_in_contact_less_eq));
    CHECK(has_flag(flags, flags_t::convert_contact_less_eq_with_same_terms));
    CHECK(!has_flag(flags, flags_t::reduce_constants));
    CHECK(!has_flag(flags, flags_t::reduce_contacts_with_constants));

    flags |= flags_t::remove_double_negation | flags_t::convert_disjunction_in_contact_less_eq | flags_t::convert_contact_less_eq_with_same_terms | flags_t::reduce_constants | flags_t::reduce_contacts_with_constants;
    CHECK(flags == flags_t::all);
    CHECK(flags != flags_t::none);

    flags_t flags2 = flags_t::remove_double_negation;
    flags_t flags3 = flags_t::convert_disjunction_in_contact_less_eq;

    flags = flags_t::reduce_contacts_with_constants;

    flags |= flags2 = flags3 = flags_t::reduce_constants;
    CHECK(flags != flags_t::reduce_constants);
    CHECK(has_flag(flags, flags_t::reduce_constants));
    CHECK(has_flag(flags, flags_t::reduce_contacts_with_constants));
    CHECK(!has_flag(flags, flags_t::convert_disjunction_in_contact_less_eq));
    CHECK(flags2 == flags_t::reduce_constants);
    CHECK(has_flag(flags2, flags_t::reduce_constants));
    CHECK(!has_flag(flags2, flags_t::remove_double_negation));
    CHECK(flags3 == flags_t::reduce_constants);
    CHECK(has_flag(flags3, flags_t::reduce_constants));
    CHECK(!has_flag(flags3, flags_t::convert_disjunction_in_contact_less_eq));
}
