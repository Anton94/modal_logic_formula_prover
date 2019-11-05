#include "catch/catch.hpp"

#include "library.h"
#include "parser_API.h"
#include "visitor.h"
#include "formula_mgr.h"

namespace
{

template <typename T1, typename T2 = T1>
void check(const std::string& input_formula, const std::string& expected_formated_output, bool check_formula_mgr_string_representation = true, formula_mgr::formula_refiners refines_flags = formula_mgr::formula_refiners::none)
{
    formula_mgr f;
    parser_error_info e;
    auto ast = parse_from_input_string(input_formula.c_str(), e);
    CHECK(ast);

    T1 visitor1;
    ast->accept(visitor1);
    T2 visitor2;
    ast->accept(visitor2);

    std::stringstream ast_printed;
    VPrinter printer(ast_printed);
    ast->accept(printer);
    CHECK(ast_printed.str() == expected_formated_output);

    CHECK(f.build(input_formula, refines_flags));

    if(check_formula_mgr_string_representation)
    {
        std::stringstream formula_mgr_printed;
        formula_mgr_printed << f;
        CHECK(formula_mgr_printed.str() == expected_formated_output);
    }
}

void check_reduce_constants(const std::string& input_formula, const std::string& expected_formated_output)
{
    check<VReduceConstants>(input_formula, expected_formated_output, true, formula_mgr::formula_refiners::reduce_constants);
}

void check_contacts_with_constant_terms(const std::string& input_formula, const std::string& expected_formated_output)
{
    check<VReduceConstants, VConvertContactsWithConstantTerms>(input_formula, expected_formated_output, true, formula_mgr::formula_refiners::reduce_contacts_with_constants | formula_mgr::formula_refiners::reduce_constants);
}

}

TEST_CASE("AST_VReduceConstants base rules", "[AST_Visitors]")
{
    check_reduce_constants("~T", "F");
    check_reduce_constants("~F", "T");

    check_reduce_constants("T | T", "T");
    check_reduce_constants("T | F", "T");
    check_reduce_constants("F | T", "T");
    check_reduce_constants("F | F", "F");
    check_reduce_constants("T & T", "T");
    check_reduce_constants("T & F", "F");
    check_reduce_constants("F & T", "F");
    check_reduce_constants("F & F", "F");

    check_reduce_constants("T | C(x,x)", "T");
    check_reduce_constants("C(x,x) | T", "T");
    check_reduce_constants("F | C(x,x)", "C(x, x)");
    check_reduce_constants("C(x,x) | F", "C(x, x)");
    check_reduce_constants("T & C(x,x)", "C(x, x)");
    check_reduce_constants("C(x,x) & T", "C(x, x)");
    check_reduce_constants("F & C(x,x)", "F");
    check_reduce_constants("C(x,x) & F", "F");

    check_reduce_constants("0=0", "T");
    check_reduce_constants("1=0", "F");

    check_reduce_constants("<=(0, a)", "T");
    check_reduce_constants("<=(0, 0)", "T");
    check_reduce_constants("<=(a, 1)", "T");
    check_reduce_constants("<=(1, 1)", "T");
    check_reduce_constants("<=(0, 1)", "T");

    check_reduce_constants("C(0, 0)", "F");
    check_reduce_constants("C(0, a)", "F");
    check_reduce_constants("C(a, 0)", "F");
    check_reduce_constants("C(1, 1)", "T");

    check_reduce_constants("C(-1, X)", "F"); // C(0,X) -> F
    check_reduce_constants("C(-0, X)", "C(1, X)");

    check_reduce_constants("C(1 * 1, X)", "C(1, X)");
    check_reduce_constants("C(1 + 1, X)", "C(1, X)");
    check_reduce_constants("C(0 * 0, X)", "F"); // C(0,X) -> F
    check_reduce_constants("C(0 + 0, X)", "F"); // C(0,X) -> F

    check_reduce_constants("C(t * 1, X)", "C(t, X)");
    check_reduce_constants("C(1 * t, X)", "C(t, X)");
    check_reduce_constants("C(t * 0, X)", "F"); // C(0,X) -> F
    check_reduce_constants("C(0 * t, X)", "F"); // C(0,X) -> F

    check_reduce_constants("C(t + 1, X)", "C(1, X)");
    check_reduce_constants("C(1 + t, X)", "C(1, X)");
    check_reduce_constants("C(t + 0, X)", "C(t, X)");
    check_reduce_constants("C(0 + t, X)", "C(t, X)");
}

TEST_CASE("AST_VReduceConstants complex", "[AST_Visitors]")
{
    check_reduce_constants("~~T", "T");
    check_reduce_constants("~~~~~T", "F");
    check_reduce_constants("~~F", "F");
    check_reduce_constants("~~~F", "T");

    check_reduce_constants("~~~~F | ~(~~T & ~~F)", "T");
    check_reduce_constants("~~~~F | ~(~T & ~F)", "T");
    check_reduce_constants("~~~~F | ~(~~T & ~F)", "F");

    check_reduce_constants("T & (F | T)", "T");
    check_reduce_constants("F | (F | T)", "T");
    check_reduce_constants("(1 + 0) * (0 + 0)=0", "T");
    check_reduce_constants("(1 + 0) * (0 + 1)=0", "F");

    check_reduce_constants("C((1 + 0) * 1, 1)", "T");
    check_reduce_constants("C(0 + (1 * 1), a)", "C(1, a)");
    check_reduce_constants("C((1 + 0) * 1, 1)", "T");

    check_reduce_constants("C((1 + X) * 1, ((X + 1) * 1) * Y)", "C(1, Y)");
    check_reduce_constants("C((1 + X) * 1, ((X + 1) * 1) * Y) & (T | C((1 + X) * 1, ((X + 1) * 1) * Y))", "C(1, Y)");
    check_reduce_constants("C((1 + X) * 1, ((X + 1) * 1) * Y) | (T | C((1 + X) * 1, ((X + 1) * 1) * Y))", "T");

    check_reduce_constants("(<=(0, X) & T) | C(X,Y * 1) & <=m(0 + m, yZ * 1)", "T");
    check_reduce_constants("(<=(0, X) & T) & C(X,Y * 1) & <=m(0 + m, yZ * 1)", "(C(X, Y) & <=m(m, yZ))");
    check_reduce_constants("(<=(0, X) & F) | C(X,Y * 1) & <=m(0 + m, yZ * 1)", "(C(X, Y) & <=m(m, yZ))");

    check_reduce_constants("(<=(0, X) & F) | C(X,Y * 1) & <=m(0 + m, yZ * 1)", "(C(X, Y) & <=m(m, yZ))");
    check_reduce_constants("(<=(0, X) & F) | C(X,Y * 1) & <=m(0 + m, yZ + 1)", "(C(X, Y) & <=m(m, 1))");
    check_reduce_constants("(<=(0, X) & F) | C(X,Y * 1) & <=(0 + m, yZ + 1)", "C(X, Y)");

    check_reduce_constants("((<=(0, X) & F) | C(X,Y * 1) & <=(0 + m, yZ + 1)) | (1 + 0) * (0 + 1)=0", "C(X, Y)");
    check_reduce_constants("((<=(0, X) & F) | C(X,Y * 1) & <=(0 + m, yZ + 1)) | (X + -Y) * (1 + 0) * (0 + 1)=0", "(C(X, Y) | ((X + -Y))=0)");
}

TEST_CASE("AST_VConvertContactsWithConstantTerms base rules", "[AST_Visitors]")
{
    check_contacts_with_constant_terms("C(a, 1)", "~(a)=0");
    check_contacts_with_constant_terms("C(1, a)", "~(a)=0");

    check_contacts_with_constant_terms("C(a + b * -z, 1)", "~((a + (b * -z)))=0");
    check_contacts_with_constant_terms("C(1, -X + -Z)", "~((-X + -Z))=0");
}

TEST_CASE("AST_VConvertContactsWithConstantTerms complex", "[AST_Visitors]")
{
    check_contacts_with_constant_terms("C(a + b * -z, 1 + Z * 1 + 1)", "~((a + (b * -z)))=0");
    check_contacts_with_constant_terms("C(1 + Z * 1 + 0, -X + -Z)", "~((-X + -Z))=0");

    check_contacts_with_constant_terms("~C(1 + Z * 1 + 0, -X + -Z) | ~C(1, Y) & <=m(m,M)", "(~~((-X + -Z))=0 | (~~(Y)=0 & <=m(m, M)))");
    check_contacts_with_constant_terms("<=m(X, Y) & <=m(m, m) | ~(C(1, Z + V) | C(-X, 1)) & <=m(m, m) & C(-Z + X, 1)",
                                       "((<=m(X, Y) & <=m(m, m)) | ((~(~((Z + V))=0 | ~(-X)=0) & <=m(m, m)) & ~((-Z + X))=0))");
}
