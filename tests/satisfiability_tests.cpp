#include "catch/catch.hpp"

#include "library.h"

void is_satisfiable(const char* input_f, bool has_satisfiable_model, bool has_satisfiable_connected_model, bool has_satisfiable_measured_model,
                    bool run_slow_models = true)
{
    formula_mgr f;
    CHECK(f.build(input_f));

    model m;
    CHECK(f.is_satisfiable(m) == has_satisfiable_model);

    if(has_satisfiable_model)
    {
        CHECK(f.is_model_satisfiable(m));
    }

    connected_model connected_m;
    CHECK(f.is_satisfiable(connected_m) == has_satisfiable_connected_model);
    if(has_satisfiable_connected_model)
    {
        CHECK(f.is_model_satisfiable(connected_m));
    }

    if(run_slow_models)
    {
        measured_model measured_m;
        CHECK(f.is_satisfiable(measured_m) == has_satisfiable_measured_model);

        if(has_satisfiable_measured_model)
        {
            CHECK(f.is_model_satisfiable(measured_m));
        }


        basic_bruteforce_model brute_force_model;
        CHECK(f.brute_force_evaluate_with_points_count(brute_force_model) == has_satisfiable_model);
        CHECK(f.is_satisfiable(brute_force_model) == has_satisfiable_model);

        if(has_satisfiable_model)
        {
            CHECK(f.is_model_satisfiable(brute_force_model));
        }
    }
}

TEST_CASE("satisfiable 0", "[satisfiability]")
{
    is_satisfiable("C(a,b+d) & T & ~(<=(a,a)) | <=m(m,x + j * -t)", true, true, true);
}

TEST_CASE("satisfiable 1", "[satisfiability]")
{
    is_satisfiable("C(a, b) | ~C(a,b)", true, true, true);
}

TEST_CASE("satisfiable 2", "[satisfiability]")
{
    is_satisfiable("C(a, b)", true, true, true);
}

TEST_CASE("satisfiable 3", "[satisfiability]")
{
    is_satisfiable("C(a, b) & ~C(a,b)", false, false, false);
}

TEST_CASE("satisfiable 3.1", "[satisfiability]")
{
    // C(a, b) & ~C(a,c)
    is_satisfiable("C(a, b) & ~C(a,c)", true, true, true);
}

TEST_CASE("satisfiable evaluation with constant 0", "[satisfiability]")
{
    is_satisfiable("C(1, b) & ~C(a, 0)", true, true, true);
}

TEST_CASE("satisfiable evaluation with constant 1", "[satisfiability]")
{
    is_satisfiable("(C(a, b) & ~C(c, 0)) & <=(1, 0)", false, false, false);
}

TEST_CASE("satisfiable evaluation with constant 1.1", "[satisfiability]")
{
    is_satisfiable("(C(a, b) & ~C(c, 0)) & <=(0, 1)", true, true, true);
}

TEST_CASE("satisfiable evaluation with constant 2", "[satisfiability]")
{
    is_satisfiable("(C(a, b) & ~C(c, 1)) & <=(1, a)", true, true, true);
}

TEST_CASE("satisfiable evaluation with constant 3", "[satisfiability]")
{
    is_satisfiable("(C(a, b) & ~C(c, 1)) & T", true, true, true);
}

TEST_CASE("satisfiable evaluation with constant 4", "[satisfiability]")
{
    is_satisfiable("(C(a, b) & ~C(c, 1)) & F", false, false, false);
}

TEST_CASE("satisfiable evaluation with constant 5", "[satisfiability]")
{
    is_satisfiable("(C(a, b) & ~C(c, 1)) | F", true, true, true);
}

TEST_CASE("satisfiable evaluation with constant 6", "[satisfiability]")
{
    is_satisfiable("(C(0, b) & ~C(c, 1)) | F", false, false, false);
}

TEST_CASE("satisfiable evaluation with constant 7", "[satisfiability]")
{
    is_satisfiable("(C(0, b) & ~C(c, 1)) | T", true, true, true);
}

TEST_CASE("satisfiable 4", "[satisfiability]")
{
    is_satisfiable("(C(a, b) | <=(a, b)) | ~C(a,b)", true, true, true);
}

TEST_CASE("satisfiable 5", "[satisfiability]")
{
    is_satisfiable("(C(a, b) | <=(a, b)) | (~C(a,b) & <=(a,b))", true, true, true);
}

TEST_CASE("satisfiable 6", "[satisfiability]")
{
    is_satisfiable("C(a, b) & ~C(a, c)", true, true, true);
}

TEST_CASE("satisfiable 7", "[satisfiability]")
{
    is_satisfiable("(C(a, b) | ~C(a, b)) & (C(a,b) | ~C(a,b))", true, true, true);
}

TEST_CASE("satisfiable 8", "[satisfiability]")
{
    is_satisfiable("(C(a, b) | ~C(a, b)) & (C(a,b) | ~C(a,c))", true, true, true);
}

TEST_CASE("satisfiable 9", "[satisfiability]")
{
    is_satisfiable("C(a, -b) & ~C(a, b + h)", true, true, true);
}

TEST_CASE("satisfiable 10", "[satisfiability]")
{
    is_satisfiable("C(a * c, (-b) + h) & ~C(a * c, (-b) + h)", false, false, false);
}

TEST_CASE("satisfiable 11", "[satisfiability]")
{
    is_satisfiable("(C(a, b) & (C(f, h) & C(j, k))) & ~((C(a, b) & C(f, h)) & C(j, k))", false, false, false, false);
}

TEST_CASE("satisfiable 12", "[satisfiability]")
{
    is_satisfiable("(C(a, b) & (C(f, h) & C(j, k))) & ~((C(a, b) & C(f, h)) & C(j, J))", true, true, true, false);
}

TEST_CASE("satisfiable 13", "[satisfiability]")
{
    is_satisfiable("<=(a,b) & <=(b,a)", true, true, true);
}

TEST_CASE("satisfiable 14", "[satisfiability]")
{
    is_satisfiable("<=(a,b) & ~(<=(b,a))", true, true, true);
}

TEST_CASE("satisfiable 15", "[satisfiability]")
{
    is_satisfiable("<=(a,b) & ~(<=(a,b))", false, false, false);
}

TEST_CASE("satisfiable 16", "[satisfiability]")
{
    is_satisfiable("(<=(a,b) & (~(<=(b,a)))) | (<=(a,b))", true, true, true);
}

TEST_CASE("satisfiable 17", "[satisfiability]")
{
    is_satisfiable("(<=(a,b) & (~(<=(a,b)))) | (C(a,b) & (~C(a,b)))", false, false, false);
}

TEST_CASE("satisfiable 18", "[satisfiability]")
{
    is_satisfiable("(<=(a,b) & (~(<=(a,b)))) | (C(a,b) & (~C(b,b)))", false, false, false);
}

TEST_CASE("satisfiable 18.1", "[satisfiability]")
{
    is_satisfiable("(<=(a,b) & (~(<=(a,b)))) | (C(a,b) & (~C(-b,b)))", true, true, true);
}

TEST_CASE("satisfiable 18.2", "[satisfiability]")
{
    is_satisfiable("(C(a,b) & ~C(b,b)) | (C(a,b) & ~C(-b,b))", true, true, true);
}

TEST_CASE("satisfiable 19", "[satisfiability]")
{
    is_satisfiable("~(C(x * -z, b * -k) | C(x * -z, c)) & (~C(-b,b) & (~<=(x, z) & ~<=(b, k)))", true, true,
                   true, false);
}

TEST_CASE("satisfiable 20", "[satisfiability]")
{
    is_satisfiable("~C(a,a)", true, true, true);
}

TEST_CASE("satisfiable 21", "[satisfiability]")
{
    is_satisfiable("~C(a,a) | ~C(-a, -a)", true, true, true);
}

TEST_CASE("satisfiable 22", "[satisfiability]")
{
    is_satisfiable("T", true, true, true);
}

TEST_CASE("satisfiable 23", "[satisfiability]")
{
    is_satisfiable("F", false, false, false);
}

TEST_CASE("satisfiable 24", "[satisfiability]")
{
    is_satisfiable("~C(a + 1, a)", true, true, true);
}

TEST_CASE("satisfiable 25", "[satisfiability]")
{
    is_satisfiable("~C(a + 1, -a)", true, true, true);
}

TEST_CASE("satisfiable but with at least two points for the measured model", "[satisfiability]")
{
    is_satisfiable("~a=0 & ~<=m(-a, a)", true, true, true);
}

TEST_CASE("satisfiable but with at least two points for the measured model 2", "[satisfiability]")
{
    is_satisfiable("~a=0 & <=m(a, 0)", true, true, false);
}

TEST_CASE("satisfiable but with at least two points for the measured model 3", "[satisfiability]")
{
    is_satisfiable("~a=0 & ~<=m(1, a)", true, true, true);
}

TEST_CASE("satisfiable models except a measured model","[satisfiability]")
{
    is_satisfiable("<=m(a, b * c) & ~<=m(a * b, c)", true, true, false);
}

TEST_CASE("satisfiable evaluation of path 1", "[satisfiability]")
{
    is_satisfiable("~C(x * -z,b) & (~C(-b,b) & ~<=(x, z))", true, true, true);
}

TEST_CASE("satisfiable evaluation of path 2", "[satisfiability]")
{
    is_satisfiable("~C(x * -z, (b * -k) + c) & (~C(-b,b) & (~<=(x, z) & ~<=(b, k)))", true, true, true, false);
}

TEST_CASE("satisfiable evaluation of path 3", "[satisfiability]")
{
    is_satisfiable("~C(x * -z, (b * -k) * c) & (~C(-b,b) & (~<=(x, z) & ~<=(b, k)))", true, true, true, false);
}

TEST_CASE("satisfiable evaluation of path 4", "[satisfiability]")
{
    is_satisfiable("(C(a,a) & C(d,d)) & ~C(a,d)", true, true, true);
}

TEST_CASE("satisfiable evaluation of path 5", "[satisfiability]")
{
    is_satisfiable("((C(a,a) & C(d,d)) & ~C(a,d)) & ((n * m) * -o = 0)", true, true, true, false);
}

TEST_CASE("satisfiable evaluation of path 6", "[satisfiability]")
{
    is_satisfiable("C(a,b) & C(b,c) & <=(a,b) & ~C(a,c)", true, true, true);
}

TEST_CASE("satisfiable evaluation of path 7", "[satisfiability]")
{
    is_satisfiable("C(a,c) & <=(a,b) & ~C(b,c)", false, false, false);
}

TEST_CASE("satisfiable evaluation of path 8", "[satisfiability]")
{
    is_satisfiable("(C(a,a) & C(d,d)) & (~C(a,d) & ((m*n)*-o) = 0)", true, true, true, false);
}

TEST_CASE("satisfiable with constants 1", "[satisfiability]")
{
    is_satisfiable("T", true, true, true);
}

TEST_CASE("satisfiable with constants 2", "[satisfiability]")
{
    is_satisfiable("F", false, false, false);
}

TEST_CASE("satisfiable with contact rule 1", "[satisfiability]")
{
    is_satisfiable("<=(a, b) & C(a * (-b), c)", false, false, false);
}

TEST_CASE("satisfiable with contact rule 1.5", "[satisfiability]")
{
    is_satisfiable("C(a * (-b), c) & <=(a, b)", false, false, false);
}

TEST_CASE("satisfiable with contact rule 2", "[satisfiability]")
{
    is_satisfiable("<=(a, c) & C(a * (-b), c)", true, true, true);
}

TEST_CASE("satisfiable with contact rule 3", "[satisfiability]")
{
    is_satisfiable("<=(a, c) & (C(a * (-c), d) & C(a * (-b), c))", false, false, false);
}

TEST_CASE("satisfiable with contact rule 4", "[satisfiability]")
{
    is_satisfiable("<=(a, c) & (C(a * (-c), d) | C(a * (-b), c))", true, true, true);
}

TEST_CASE("satisfiable with contact rule 5", "[satisfiability]")
{
    is_satisfiable("(<=(a, c) | <=(a,b)) & (C(a * (-c), d) | C(a * (-b), c))", true, true, true);
}

TEST_CASE("satisfiable with contact rule 6", "[satisfiability]")
{
    is_satisfiable("(<=(a, c) & <=(a,b)) & (C(a * (-c), d) | C(a * (-b), c))", false, false, false, false);
}

TEST_CASE("satisfiable with contact rule 6.1", "[satisfiability]")
{
    is_satisfiable("~(~<=(a, c) | ~<=(a,b)) & (C(a * (-c), d) | C(a * (-b), c))", false, false, false, false);
}

TEST_CASE("satisfiable with contact rule 7", "[satisfiability]")
{
    is_satisfiable("(~<=(a,b) & (~<=(c,d) & C(a * -b, c * -d)))", true, true, true);
}

TEST_CASE("satisfiable with contact rule 8", "[satisfiability]")
{
    is_satisfiable("(~<=(a,b) & (~<=(c,d) & ~C(a * -b, c * -d)))", true, true, true);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact", "[satisfiability]")
{
    is_satisfiable("(<=(x,y) & <=(a,b)) & C(a * -b, z)", false, false, false, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact 2", "[satisfiability]")
{
    is_satisfiable("((~<=(x,y) & <=(z, t)) & ~<=(a,b)) & C(a * -b, z * -t)", false, false, false, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact 3", "[satisfiability]")
{
    is_satisfiable("((~<=(x,y) & ~<=(z, t)) & ~<=(a,b)) & C(a * -b, z * -t)", true, true, true, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact 4", "[satisfiability]")
{
    is_satisfiable("~( ~((<=(x,y) & ~<=(z, t)) & ~<=(a,b)) | C(a * -b, z * -t) )", true, true, true, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact using same terms 1",
          "[satisfiability]")
{
    is_satisfiable("C(a * -d, b * -c) & ( (C(a * -d, d) & <=(a,d)) | <=(b,c))", false, false, false, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact using same terms 2",
          "[satisfiability]")
{
    is_satisfiable("C(a * -d, b * -c) & ((C(a * -d, d) & <=(a,d)) | <=(b,x))", true, true, true, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact using same terms 2.1",
          "[satisfiability]")
{
    is_satisfiable("C(a * -d, b * -c) & ( (C(a * -d, -d) & <=(a,d)) | <=(b,x))", true, true, true, false);
}

TEST_CASE("satisfiable with contact rule - adding zero terms after adding contact using same terms 3",
          "[satisfiability]")
{
    is_satisfiable(
        "~C(a * -d, b * -c) & ( (~C(a * -d, d * -c) & (~<=(a,d) & ~<=(d,c))) | (~<=(b,c) & ~<=(a,d)) )", true,
        true, true, false);
}

TEST_CASE("satisfiable with contact rule - adding non zero term and F contact with same left/right child 1",
          "[satisfiability]")
{
    is_satisfiable("~C(a * -d, a * -d) & ~<=(a,d)", false, false, false);
}

TEST_CASE("satisfiable with contact rule - adding non zero term and F contact with same left/right child 2",
          "[satisfiability]")
{
    is_satisfiable("~<=(a,d) & ~C(a * -d, a * -d)", false, false, false);
}

TEST_CASE("satisfiable evaluation of atomic formulas when there is a constant 1", "[satisfiability]")
{
    is_satisfiable("C(a, -a) & ~(F)", true, true, true);
}

TEST_CASE("satisfiable evaluation of atomic formulas when there is a constant 2", "[satisfiability]")
{
    is_satisfiable("C(a, -a) & ~(C(e,f) & ~T)", true, true, true);
}

TEST_CASE("satisfiable evaluation of atomic formulas when there is a constant 3", "[satisfiability]")
{
    is_satisfiable("C(a, -a) & (C(f,g) | T)", true, true, true);
}

TEST_CASE("satisfiable evaluation of atomic formulas when there is a constant 4", "[satisfiability]")
{
    is_satisfiable("C(a, -a) & ~(C(f,g) & F)", true, true, true);
}

TEST_CASE("satisfiable when F disjunction has contradicting right child 1", "[satisfiability]")
{
    is_satisfiable("~(C(a,b) | T) | C(a,b)", true, true, true);
}

TEST_CASE("satisfiable when T conjunction has contradicting right child 1", "[satisfiability]")
{
    is_satisfiable("(C(a,b) & F) | ~C(a,b)", true, true, true);
}

TEST_CASE("not satisfiable with 10 variables", "[satisfiability]")
{
    is_satisfiable("C(a,b) & C(b,c) & <=(b,a) & ~C(a,c) & C(d, e) & C(f,g) & C(h, i) & C(h, j)", false, false,
                   true, false);
}

TEST_CASE("satisfiable but resetting in measured_model functionality", "[satisfiability]")
{
    is_satisfiable("~<=m(a * b, 0) & ~<=(a,b)", true, true, true);
}

TEST_CASE("<=m to check the formula's evaluate correctnes when right child is with <=m 1", "[satisfiability]")
{
    is_satisfiable("C(a,b) & ~C(a,b) | ~<=m(b,c + a)", true, true, true);
}

TEST_CASE("<=m to check the formula's evaluate correctnes when right child is with <=m 2", "[satisfiability]")
{
    is_satisfiable("C(a,b) & ~C(a,b) | <=m(b,c + a)", true, true, true);
}

TEST_CASE("<=m to check the formula's evaluate correctnes when right child is with <=m 3", "[satisfiability]")
{
    is_satisfiable("C(a,b) & <=(b,c) & C(a,c) & <=m(a,b) & ~<=m(b,c + a)", true, true, false);
}

TEST_CASE("<=m to check the formula's evaluate correctnes when right child is with <=m 4", "[satisfiability]")
{
    is_satisfiable("C(a,b) & <=(b,c) & C(a,c) & <=m(a,b) & <=m(b,c + a)", true, true, true);
}

TEST_CASE("<=m to check the formula's evaluate correctnes when right child is with <=m 5", "[satisfiability]")
{
    is_satisfiable("C(a,b) & <=(b,c) & C(a,c) & ~(<=m(a,b) & <=m(b,c + a))", true, true, true);
}

TEST_CASE("not satisfiable system 1", "[satisfiability]")
{
    is_satisfiable("C(a,b) & <=(b,c) & ~<=m(b,c + a)", true, true, false);
}

TEST_CASE("not satisfiable system 2", "[satisfiability]")
{
    is_satisfiable("C(a,b) & <=(b,c) & C(a,c) | <=m(a,b) & ~<=m(b,c + a)", true, true, true);
}

TEST_CASE("measured_model additional points 1", "[satisfiability]")
{
    is_satisfiable("~<=m(a,0)", true, true, true);
}

TEST_CASE("measured_model additional points 2", "[satisfiability]")
{
    is_satisfiable("~<=m(a,b)", true, true, true);
}

TEST_CASE("measured_model additional points 3", "[satisfiability]")
{
    is_satisfiable("~<=m(a * b, 0) & ~<=m(a,b)", true, true, true);
}

TEST_CASE("empty model satisfies the formula but there should be at least one point 1", "[satisfiability]")
{
    is_satisfiable("a=0 & -a=0", false, false, false);
}

TEST_CASE("empty model satisfies the formula but there should be at least one point 2", "[satisfiability]")
{
    is_satisfiable("~C(a,a) & ~C(-a,-a)", false, false, false);
}

TEST_CASE("empty model satisfies the formula but there should be at least one point 3", "[satisfiability]")
{
    is_satisfiable("~C(a,a) & ~C(-a,-a)", false, false, false);
}

TEST_CASE("empty model satisfies the formula but there should be at least one point 4", "[satisfiability]")
{
    is_satisfiable("~C(1, 1)", false, false, false);
}

TEST_CASE("empty model satisfies the formula but there should be at least one point 5", "[satisfiability]")
{
    is_satisfiable("(a + -a)=0", false, false, false);
}

TEST_CASE("empty model satisfies the formula but there should be at least one point 6", "[satisfiability]")
{
    is_satisfiable("~C(a + -a, b + -b)", false, false, false);
}
