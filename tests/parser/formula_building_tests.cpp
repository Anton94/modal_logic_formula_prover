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

void check_SE(const std::string& input_formula, int column, int line = 1)
{
    check(input_formula, "", false, {line, column, "syntax error"});
}

}

TEST_CASE("AST building syntax error", "[AST_building]")
{
    check_SE("", 1);
    check_SE("()", 2);
    check_SE("(())", 3);
    check_SE("(())()", 3);
    check_SE("C(a9, b0", 7);
    check_SE("C(a9,\n   b0", 4, 2);
    check_SE("C a9, b0", 3);
    check_SE("C a9, b0)", 3);
    check_SE("C(a9), b0)", 5);
    check_SE("C(a9, )", 7);
    check_SE("C( , x)", 4);
    check_SE("C( , x) | <=()", 4);
    check_SE("C(x , x) | <=()", 15);
    check_SE("C(x , x) | <=(a)", 16);
    check_SE("C(x , x) | <=(a,)", 17);
    check_SE("C(x , x) | <=(a,b", 17);
    check_SE("C(x , x) | <=(a,b(", 18);
    check_SE("C(x , x) | <=(a,b) ~", 20);
    check_SE("C(x , x) <=(a,b)", 10);

    check_SE("C(ax, bx) C(yz, ft) | C(r,f) & C(gg,asx)", 11);
    check_SE("C(ax, bx) & C(yz, ft) C(r,f) & C(gg,asx)", 23);
    check_SE("C(ax, bx) & C(yz, ft) | C(r,f) C(gg,asx)", 32);
    check_SE("C(ax, bx) -> C(yz, ft) ~C(r,f) & C(gg,asx)", 24);

    check_SE("C(ax, bx) ~ <-> C(yz, ft) ~C(r,f) & C(gg,asx)", 11);

    check_SE("(C(ax, bx) ->) C(yz, ft) ~C(r,f) & C(gg,asx)", 14);
    check_SE("(C(ax, bx) ->() C(yz, ft) ~C(r,f) & C(gg,asx)", 15);
    check_SE("(C(ax, bx) ->) C(yz, ft) ~C(r,f) & C(gg,asx)", 14);
    check_SE("(C(ax, bx) -> C(yz, ft)) | ~C(r,f) ~ & C(gg,asx)", 36);
    check_SE("(C(ax, bx) -> C(yz, ft)) | ~C(r,f) ~T & C(gg,asx)", 36);
    check_SE("(C(ax, bx) -> C(yz, ft)) | ~C(r,f) (| ~T & C(gg,asx))", 36);

    check_SE("(C(ax, bx) -> C(yz +, ft)) | ~C(r,f) | (~T & C(gg,asx))", 21);
    check_SE("(C(ax, bx) -> C(yz = -x, ft)) | ~C(r,f) | (~T & C(gg,asx))", 20);
    check_SE("(C(ax, bx) -> C(yz & -x, ft)) | ~C(r,f) | (~T & C(gg,asx))", 20);
    check_SE("(C(ax, bx) -> C(yz | -x, ft)) | ~C(r,f) | (~T & C(gg,asx))", 20);
    check_SE("(C(ax, bx) -> C(a, ft)) | ~C(r,f) | (~T & C(gg,yz + ~x))", 53);
    check_SE("(C(ax, bx) -> C(a, ft)) | ~C(r,f) | (~T & C(gg,yz + ~x))", 53);
    check_SE("(C(ax, bx) -> C(yz - x*, ft)) | ~C(r,f) | (~T & C(gg,asx))", 20);
    check_SE("(C(ax, bx) -> C(yz + x*, ft)) | ~C(r,f) | (~T & C(gg,asx))", 24);
    check_SE("(C(ax, bx) -> C(z, ft)) | ~C(r,yz + x*) | (~T & C(gg,asx))", 39);
    check_SE("(C(ax, bx) -> C(z, ft)) | ~C(*r,yz + x*) | (~T & C(gg,asx))", 30);

    check_SE("(C(ax, bx) -> C(z, ft)) | ~C(r,yz + x*) | (~0 & C(gg,asx))", 39);
    check_SE("(C(ax, bx) -> C(z, ft)) | ~C(r,yz + x) | (~0 & C(gg,asx))", 46);
    check_SE("(C(ax, bx) -> C(z, ft)) | ~C(r,yz + x) | (~1 & C(gg,asx))", 46);
    check_SE("(C(ax, bx) -> C(~z, ft)) | ~C(*r,yz + x*) | (~1 & C(gg,asx))", 17);
    check_SE("(C(ax, bx) -> C(z, ~ft)) | ~C(r,yz + x) | (~T & C(gg,asx))", 20);
    check_SE("(C(ax, ~bx) -> C(z, ~ft)) | ~C(r,yz + x) | (~T & C(gg,asx))", 8);
    check_SE("(C(~ax, ~bx) -> C(z, ~ft)) | ~C(r,yz + x) | (~T & C(gg,asx))", 4);

    check_SE("C(a9, b0) * C(e,fg)", 11);
    check_SE("C(a9, b0 * Ce,fg)", 14);
    check_SE("C(a9, T)", 7);
    check_SE("C(a9, F)", 7);
    check_SE("C(F, x)", 3);
    check_SE("C(T, x)", 3);
    check_SE("(C(ax, bx) -> C(z, ft)) | C(a9, T)", 33);
    check_SE("(C(ax, bx) -> C(z, ft)) | C(a9, F)", 33);
    check_SE("(C(ax, bx) -> C(z, ft)) | C(F, x)", 29);
    check_SE("(C(ax, bx) -> C(z, ft)) | C(T, x)", 29);
    check_SE("(C(ax, bx) -> C(T, ft)) | C(a9, 1)", 17);
    check_SE("(C(ax, bx) -> C(z, F)) | C(a9, 0)", 20);
    check_SE("(C(T, 1) -> C(z, ft)) | C(0, x)", 4);
    check_SE("(C(0, F) -> C(z, ft)) | C(1, x)", 7);
    check_SE("(C(0, F) -> C(z, ft)) | C(1, x)", 7);

    check_SE("adfsaf", 1);
    check_SE("C", 1);
    check_SE("<=", 1);
    check_SE(">=", 1);
    check_SE("C(a, <=(e,f))", 6);
    check_SE("C(a, C))", 6);
    check_SE("<=(a, C))", 7);
    check_SE("C(ax, bx) -> adfsaf", 14);
    check_SE("C(ax, bx) -> C", 14);
    check_SE("C(ax, bx) -> <=", 14);
    check_SE("C(ax, bx) -> >=", 14);
    check_SE("C(ax, bx) -> C(a, <=(e,f))", 19);
    check_SE("C(ax, bx) -> C(a, C))", 19);
    check_SE("C(ax, bx) -> <=(a, C))", 20);
    check_SE("C(ax, bx) -> =<(a, C))", 14);

    check_SE("ax * bx", 6);
    check_SE("ax * bx <= ef", 9);
    check_SE("ax * bx!=0", 8);
    check_SE("ax & ZZzZ1023", 4);
    check_SE("T & ZZzZ1023", 5);
    check_SE("T & ZZzZ1023", 5);
    check_SE("t & ZZzZ1023", 3);
    check_SE("f & ZZzZ1023", 3);
    check_SE("<=m(m,m) & (F & ZZzZ1023) -> T", 25);
    check_SE("<=m(m,m) & (ax & T) -> T", 16);

    check_SE("T && F", 4);
    check_SE("T &| F", 4);
    check_SE("T &~& F", 5);
    check_SE("T ~& F", 3);
    check_SE("~T <--> F", 4);
    check_SE("~T <-> -> F", 8);
    check_SE("<=(a,b) <-> C(e,f) | <-m(g,h)", 22);
    check_SE("<=(a,b) <-> C(e,~f) | <=m(g,h)", 17);
    check_SE("<=(a,b) <-> C(e,-f * +g) | <=m(g,h)", 22);
    check_SE("<=(a,b) <-> C(-f * +g,a) | <=m(g,h)", 20);
    check_SE("<=(a,-f * +gb) <-> C(b,a) | <=m(g,-f * +g)", 11);
    check_SE("<=(-f * +gb,b) <-> C(b,a) | <=m(g,-f * +g)", 9);
    check_SE("<=(x,b) <-> C(b,a) | <=m(g,-f * +g)", 33);
    check_SE("<=(x,b) <-> C(b,a) | <=m(-f * +g,qx1)", 31);
    check_SE("<=(x,b) <-> C(b,a) | <=m(-f * -g,qx1) C(e,f)", 39);
    check_SE("<=(x,b) <-> C(b,a | b) | <=m(-f * -g,qx1) C(e,f)", 19);
    check_SE("<=(x,b) <-> C(b,a & b) | <=m(-f * -g,qx1) C(e,f)", 19);
    check_SE("<=(x,b) --> C(b,a & b) | <=m(-f * -g,qx1) C(e,f)", 9);
    check_SE("<=(x,b) - C(b,a & b) | <=m(-f * -g,qx1) C(e,f)", 9);

    check_SE("<=(x,b) <- C(b,a * (b + ~j)) | <=m(-f * -g,qx1) C(e,f) ads", 9);
    check_SE("<=(x,b) <> C(b,a * (b + ~j)) | <=m(-f * -g,qx1) C(e,f) asd", 9);
    check_SE("<=(x,b) <-> C(b,a * (b + ~j)) | <=m(-f * -g,qx1) C(e,f)asd  as", 26);
    check_SE("<=(x,b)ax <-> C(b,a * (b + ~j)) | <=m(-f * -g,qx1) C(e,f)asd  as", 8);
    check_SE("<=(x,b) <-> C(b,a * (b + -j)) | <=m(-f * -g,qx1) | C(e,f)asd  as", 58);
    check_SE("<=(x,b) <-> C(b,a * (b + -j)) | <=m(-f * -g,qx1) | C(e,f) as", 59);
    check_SE("<=(x,b) <-> C(b,a * (b + -j)asd) | <=m(-f * -g,qx1) | C(e,f) as", 29);
    check_SE("<=(x,b) <-> C(b,a * as(b + -j)asd) | <=m(-f * -g,qx1) | C(e,f) as", 23);
    check_SE("<=(C,b12d) <-> C(b,a * as(b + -j)asd) | <=m(-f * -g,qx1) | C(e,f) as", 4);
    check_SE("<=(Cx,b12d) <-> C(b,a * as(b + -j)asd) | <=m(-f * -g,qx1) | C(e,f) as", 27);
    check_SE("<=(C(x,y),b12d) <-> C(b,a * as(b + -j)asd) | <=m(-f * -g,qx1) | C(e,f) as", 4);
    check_SE("<=(X11C),b12d) <-> C(b ax,a * as(b + -j)asd) | <=m(-f * -g,qx1) | C(e,f) as", 8);
    check_SE("<=(X11C,b12d) <-> C(b ax,a * as(b + -j)asd) | <=m(-f * -g,qx1) | C(e,f) as", 23);
    check_SE("<=(X11C,b12d) <-> C(b_ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 22);
    check_SE("<=(X11C,b12d) <-> C(b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 79);
    check_SE("<=(X11C,b12d) <-> Ca,b) & (b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 21);
    check_SE("<=(X11C,b12d) <-> <=a,b) & (b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 21);
    check_SE("<=(X11C,b12d) <-> <=ma,b) & (b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 22);
    check_SE("<=(X11C,b12d) <-> C(ab) & (b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 23);
    check_SE("<=(X11C,b12d) <-> <=(ab) & (b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 24);
    check_SE("<=(X11C,b12d) <-> <=m(ab) & (b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 25);
    check_SE("<=(X11C,b12d) <-> (a * b =0) & C(b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 92);
    check_SE("<=(X11C,b12d) <-> ~((a * b)=0) & C(b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 94);
    check_SE("<=(X11C,b12d) <-> -((a * b)=0) & C(b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 28);
    check_SE("<=(X11C,b12d) <-> -(a * b)=0) & C(b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 29);
    check_SE("<=(X11C,b12d) <-> (a * ~b =0) & C(b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 24);
    check_SE("<=(X11C,b12d) <-> (a * -b =0) & C(b1ax,a * as * (b + -j) * asd) | <=m(-f * -g,qx1) | C(e,f) as", 93);
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

