#include "catch/catch.hpp"

#include "library.h"
#include "parser_API.h"
#include "visitor.h"
#include "formula_mgr.h"

namespace
{

template <typename T>
void check(const std::string& input_formula, const std::string& expected_formated_output, bool check_formula_mgr_string_representation = true, formula_mgr::formula_refiners refines_flags = formula_mgr::formula_refiners::none)
{
    formula_mgr f;
    parser_error_info e;
    auto ast = parse_from_input_string(input_formula.c_str(), e);
    CHECK(ast);

    T visitor;
    ast->accept(visitor);

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

}

TEST_CASE("AST_VReduceConstants base rules", "[AST_VReduceConstants]")
{
    check_reduce_constants("~T", "F");
    check_reduce_constants("~~T", "T");
    check_reduce_constants("~~~~~T", "F");
    check_reduce_constants("~F", "T");
    check_reduce_constants("~~F", "F");
    check_reduce_constants("~~~F", "T");
    check_reduce_constants("T | T", "T");
    check_reduce_constants("T | F", "T");
    check_reduce_constants("F | T", "T");
    check_reduce_constants("F | F", "F");
    check_reduce_constants("T & T", "T");
    check_reduce_constants("T & F", "F");
    check_reduce_constants("F & T", "F");
    check_reduce_constants("F & F", "F");

    check_reduce_constants("T & T", "T");
    check_reduce_constants("T & F", "F");
    check_reduce_constants("F & T", "F");
    check_reduce_constants("F & F", "F");


    check_reduce_constants("C(0, 0)", "F");
    check_reduce_constants("C(0, a)", "F");
    check_reduce_constants("C(a, 0)", "F");
    check_reduce_constants("C(1, 1)", "T");

    check_reduce_constants("<=(0, a)", "T");
    check_reduce_constants("<=(0, 0)", "T");
    check_reduce_constants("<=(a, 1)", "T");
    check_reduce_constants("<=(1, 1)", "T");
    check_reduce_constants("<=(0, 1)", "T");

    check_reduce_constants("0=0", "T");
    check_reduce_constants("1=0", "F");
    check_reduce_constants("(1 + 0) * (0 + 0)=0", "T");
    check_reduce_constants("(1 + 0) * (0 + 1)=0", "F");

    check_reduce_constants("C((1 + 0) * 1, 1)", "T");
    check_reduce_constants("C(0 + (1 * 1), a)", "C(1, a)");
    check_reduce_constants("C((1 + 0) * 1, 1)", "T");

    // TODO add the rest of the basic cases
}

TEST_CASE("AST_VReduceConstants complex", "[AST_VReduceConstants]")
{
    // TODO: add complex tests

    check_reduce_constants("T & (F | T)", "T");
    check_reduce_constants("F | (F | T)", "T");
}
