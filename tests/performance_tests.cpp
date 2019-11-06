/*
 *
 *  This tests are not ran with the rest of the tests.
 *  Run the test app with the following arguments: -d yes "[performance]"
 *  Consider also building in Release, because they take some time.
 *
 */
#include "catch/catch.hpp"

#include "library.h"

template<typename T>
void is_satisfiable(const char* input_f, bool has_satisfiable_model)
{
    formula_mgr f;
    CHECK(f.build(input_f));

    T m;
    CHECK(f.is_satisfiable(m) == has_satisfiable_model);
}

#define PERF_TEST_CASE(name, formula, has_model, has_connected_model, run_slow_model, has_slow_model) \
TEST_CASE(name" - model", "[!hide][performance]")                                                     \
{                                                                                                     \
    is_satisfiable<model>(formula, has_model);                                                        \
}                                                                                                     \
TEST_CASE(name" - connected_model", "[!hide][performance]")                                           \
{                                                                                                     \
    is_satisfiable<connected_model>(formula, has_connected_model);                                    \
}                                                                                                     \
TEST_CASE(name" - slow_model - Enabled["#run_slow_model"] " , "[!hide][performance]")                 \
{                                                                                                     \
    if(run_slow_model)                                                                                \
        is_satisfiable<slow_model>(formula, has_slow_model);                                          \
}

PERF_TEST_CASE("not satisfiable with 6 variables 4 atomic formulas",
               "C(a * x, b * y) & C(b * y, c * z) & <=(b * y, a * x * v) & ~C(a * x, c * z)",
               false, false, true, false);

PERF_TEST_CASE("not satisfiable with 7 variables 4 atomic formulas",
               "C(a * x, b * y) & C(b * y, c * z * t) & <=(b * y, a * x * v) & ~C(a * x, c * z * t)",
               false, false, true, false);

PERF_TEST_CASE("not satisfiable with 10 variables 8 atomic formulas",
               "C(a,b) & C(b,c) & <=(b,a) & ~C(a,c) & C(d, e) & C(f,g) & C(h, i) & C(h, j)",
               false, false, false, false);
