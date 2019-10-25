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
