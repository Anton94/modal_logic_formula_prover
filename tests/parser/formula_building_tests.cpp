#include "catch/catch.hpp"

#include "library.h"
#include "parser_API.h"
#include "visitor.h"
#include "formula_mgr.h"

namespace
{

void check(const std::string& input_formula, const std::string& expected_formated_output, bool check_formula_mgr_string_representation = true, const parser_error_info& expected_error = {})
{
    formula_mgr f;
    parser_error_info e;
    auto ast = parse_from_input_string(input_formula.c_str(), e);
    if(!ast)
    {
        CHECK(expected_error.line == e.line);
        CHECK(expected_error.column == e.column);
        CHECK(expected_error.msg == e.msg);
        CHECK(!f.build(input_formula));
        return;
    }

    CHECK(expected_error.msg.empty());

    std::stringstream ast_printed;
    VPrinter printer(ast_printed);
    ast->accept(printer);
    CHECK(ast_printed.str() == expected_formated_output);

    CHECK(f.build(input_formula, formula_mgr::formula_refiners::none));

    if(check_formula_mgr_string_representation)
    {
        std::stringstream formula_mgr_printed;
        formula_mgr_printed << f;
            CHECK(formula_mgr_printed.str() == expected_formated_output);
    }
}

}

TEST_CASE("AST building syntax error", "[AST_building]")
{
    check("", "", false, {1, 1, "syntax error"});
    check("()", "", false, {1, 2, "syntax error"});
    check("(())", "", false, {1, 3, "syntax error"});
    check("(())()", "", false, {1, 3, "syntax error"});
    check("C(a9, b0", "", false, {1, 7, "syntax error"});
    check("C(a9,\n   b0", "", false, {2, 4, "syntax error"});
    check("C a9, b0)", "", false, {1, 3, "syntax error"});

    check("C(ax, bx) C(yz, ft) | C(r,f) & C(gg,asx)", "", false, {1, 11, "syntax error"});
    check("C(ax, bx) & C(yz, ft) C(r,f) & C(gg,asx)", "", false, {1, 23, "syntax error"});
    check("C(ax, bx) & C(yz, ft) | C(r,f) C(gg,asx)", "", false, {1, 32, "syntax error"});
    check("C(ax, bx) -> C(yz, ft) ~C(r,f) & C(gg,asx)", "", false, {1, 24, "syntax error"});

    check("C(ax, bx) ~ <-> C(yz, ft) ~C(r,f) & C(gg,asx)", "", false, {1, 11, "syntax error"});

    check("(C(ax, bx) ->) C(yz, ft) ~C(r,f) & C(gg,asx)", "", false, {1, 14, "syntax error"});
    check("(C(ax, bx) ->() C(yz, ft) ~C(r,f) & C(gg,asx)", "", false, {1, 15, "syntax error"});
    check("(C(ax, bx) ->) C(yz, ft) ~C(r,f) & C(gg,asx)", "", false, {1, 14, "syntax error"});
    check("(C(ax, bx) -> C(yz, ft)) | ~C(r,f) ~ & C(gg,asx)", "", false, {1, 36, "syntax error"});
    check("(C(ax, bx) -> C(yz, ft)) | ~C(r,f) ~T & C(gg,asx)", "", false, {1, 36, "syntax error"});
    check("(C(ax, bx) -> C(yz, ft)) | ~C(r,f) (| ~T & C(gg,asx))", "", false, {1, 36, "syntax error"});

    check("(C(ax, bx) -> C(yz +, ft)) | ~C(r,f) | (~T & C(gg,asx))", "", false, {1, 21, "syntax error"});
    check("(C(ax, bx) -> C(yz = -x, ft)) | ~C(r,f) | (~T & C(gg,asx))", "", false, {1, 20, "syntax error"});
    check("(C(ax, bx) -> C(yz & -x, ft)) | ~C(r,f) | (~T & C(gg,asx))", "", false, {1, 20, "syntax error"});
    check("(C(ax, bx) -> C(yz | -x, ft)) | ~C(r,f) | (~T & C(gg,asx))", "", false, {1, 20, "syntax error"});
    check("(C(ax, bx) -> C(a, ft)) | ~C(r,f) | (~T & C(gg,yz + ~x))", "", false, {1, 53, "syntax error"});
    check("(C(ax, bx) -> C(a, ft)) | ~C(r,f) | (~T & C(gg,yz + ~x))", "", false, {1, 53, "syntax error"});
    check("(C(ax, bx) -> C(yz - x*, ft)) | ~C(r,f) | (~T & C(gg,asx))", "", false, {1, 20, "syntax error"});
    check("(C(ax, bx) -> C(yz + x*, ft)) | ~C(r,f) | (~T & C(gg,asx))", "", false, {1, 24, "syntax error"});
    check("(C(ax, bx) -> C(z, ft)) | ~C(r,yz + x*) | (~T & C(gg,asx))", "", false, {1, 39, "syntax error"});
    check("(C(ax, bx) -> C(z, ft)) | ~C(*r,yz + x*) | (~T & C(gg,asx))", "", false, {1, 30, "syntax error"});
}

TEST_CASE("AST building constants", "[AST_building]")
{
    check("T", "T");
    check("F", "F");
    check("F | F", "(F | F)");
    check("(F) | (F)", "(F | F)");
    check("((F) | F)", "(F | F)");
    check("((F) | (F))", "(F | F)");
}

TEST_CASE("AST term building operations associativity and priority", "[AST_building]")
{
    check("C(a + b + c + d + -e * f * ax * a * c, a + b * c + d * -e + -h * d)",
          "C(((((a + b) + c) + d) + ((((-e * f) * ax) * a) * c)), (((a + (b * c)) + (d * -e)) + (-h * d)))");
}

TEST_CASE("AST building operations associativity and priority 1", "[AST_building]")
{
    check("C(a + b + c + d + -e * f * ax * a * c, a + b * c + d * -e + -h * d) |"
          "<=m(ax, xz * -d * -z) |"
          "ax + -b * -c * -d * -e + f + h = 0",
          "((C(((((a + b) + c) + d) + ((((-e * f) * ax) * a) * c)), (((a + (b * c)) + (d * -e)) + (-h * d))) | "
          "<=m(ax, ((xz * -d) * -z))) | "
          "((((ax + (((-b * -c) * -d) * -e)) + f) + h))=0)");
}

TEST_CASE("AST building operations associativity and priority 2", "[AST_building]")
{
    check("<=m(m,m) | C(ax,-d) | C(-d,ax) | <=m(b,c) | T | F | <=m(a,b) & C(0,1) & ~(<=m(-f,--g + 0) | <=m(ymv,v)) | F & T & F & <=m(x,v)",
          "(((((((<=m(m, m) | C(ax, -d)) | C(-d, ax)) | <=m(b, c)) | T) | F) | ((<=m(a, b) & C(0, 1)) & ~(<=m(-f, (--g + 0)) | <=m(ymv, v)))) | (((F & T) & F) & <=m(x, v)))");
}

TEST_CASE("AST building operations associativity and priority 3", "[AST_building]")
{
    check("C(a9, b0) & C(e,f) -> C(ax,a) | ax * -b = 0 <-> <=m(e,f) -> C(z,zz)",
          "((((C(a9, b0) & C(e, f)) -> (C(ax, a) | ((ax * -b))=0)) <-> <=m(e, f)) -> C(z, zz))", false);
}

TEST_CASE("AST building operations associativity and priority 4", "[AST_building]")
{
    check("C(a9, b0) & C(e,f) -> C(ax,a) | (ax * -b = 0 <-> <=m(e,f)) -> C(z,zz)",
          "(((C(a9, b0) & C(e, f)) -> (C(ax, a) | (((ax * -b))=0 <-> <=m(e, f)))) -> C(z, zz))", false);
}

