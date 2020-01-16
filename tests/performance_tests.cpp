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

#define PERF_TEST_CASE(name, formula, has_model, has_connected_model, run_measured_model, has_measured_model) \
TEST_CASE(name" - model", "[!hide][performance]")                                                             \
{                                                                                                             \
    is_satisfiable<model>(formula, has_model);                                                                \
}                                                                                                             \
TEST_CASE(name" - connected_model", "[!hide][performance]")                                                   \
{                                                                                                             \
    is_satisfiable<connected_model>(formula, has_connected_model);                                            \
}                                                                                                             \
TEST_CASE(name" - optimized_measured_model" , "[!hide][performance]")                                         \
{                                                                                                             \
    is_satisfiable<optimized_measured_model>(formula, has_measured_model);                                    \
}                                                                                                             \
TEST_CASE(name" - measured_model - Enabled["#run_measured_model"] " , "[!hide][performance]")                 \
{                                                                                                             \
    if(run_measured_model)                                                                                    \
        is_satisfiable<measured_model>(formula, has_measured_model);                                          \
}                                                                                                             \

PERF_TEST_CASE("not satisfiable with 6 variables 4 atomic formulas",
               "C(a * x, b * y) & C(b * y, c * z) & <=(b * y, a * x) & ~C(a * x, c * z)",
               false, false, true, false);

PERF_TEST_CASE("not satisfiable with 8 variables 4 atomic formulas",
               "C(a * x1 * x2, b * y) & C(b * y, c * z * t) & <=(b * y, a * x1 * x2) & ~C(a * x1 * x2, c * z * t)",
               false, false, true, false);

PERF_TEST_CASE("not satisfiable with 10 variables 8 atomic formulas",
               "C(a,b) & C(b,c) & <=(b,a) & ~C(a,c) & C(d, e) & C(f,g) & C(h, i) & C(h, j)",
               false, false, false, false);

PERF_TEST_CASE("not satisfiable with 4 variables 9 atomic formulas, transitive property variant 0",
               "C(a,b) & C(b,c) => C(a,c) & C(a,x) & C(x,c) => C(a,c) & ~(C(b,c) & C(c,x) => C(b,x))",
               false, false, true, false)
