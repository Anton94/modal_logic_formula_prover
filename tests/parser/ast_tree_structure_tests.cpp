#include "catch/catch.hpp"

#include "library.h"
#include "parser_API.h"

static parser_error_info default_error_info_obj;

TEST_CASE("empty string", "[AST_structure]")
{
    auto ast = parse_from_input_string("", default_error_info_obj);
    CHECK(ast.get() == nullptr);
}

TEST_CASE("atomic constant T", "[AST_structure]")
{
    auto ast = parse_from_input_string("T", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::constant_true);
}

TEST_CASE("atomic constant F", "[AST_structure]")
{
    auto ast = parse_from_input_string("F", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::constant_false);
}

TEST_CASE("atomic contact", "[AST_structure]")
{
    auto ast = parse_from_input_string("C(m,yZ123)", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::contact);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::variable);
    CHECK(left->variable == "m");
    auto right = static_cast<NTerm*>(ast->right);
    CHECK(right->op == term_operation_t::variable);
    CHECK(right->variable == "yZ123");
}

TEST_CASE("atomic contact with complex terms", "[AST_structure]")
{
    auto ast = parse_from_input_string("C(m + -x, -y * -mx)", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::contact);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::union_);
    CHECK(left->left->op == term_operation_t::variable);
    CHECK(left->left->variable == "m");
    CHECK(left->right->op == term_operation_t::complement);
    CHECK(left->right->left->op == term_operation_t::variable);
    CHECK(left->right->left->variable == "x");

    auto right = static_cast<NTerm*>(ast->right);
    CHECK(right->op == term_operation_t::intersaction);
    CHECK(right->left->op == term_operation_t::complement);
    CHECK(right->left->left->op == term_operation_t::variable);
    CHECK(right->left->left->variable == "y");
    CHECK(right->right->op == term_operation_t::complement);
    CHECK(right->right->left->op == term_operation_t::variable);
    CHECK(right->right->left->variable == "mx");
}

TEST_CASE("atomic less eq", "[AST_structure]")
{
    auto ast = parse_from_input_string("<=(m,m)", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::less_eq);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::variable);
    CHECK(left->variable == "m");
    auto right = static_cast<NTerm*>(ast->right);
    CHECK(right->op == term_operation_t::variable);
    CHECK(right->variable == "m");
}

TEST_CASE("atomic less eq with complex terms", "[AST_structure]")
{
    auto ast = parse_from_input_string("<=(m + -x42, m * (-mx + -42x))", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::less_eq);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::union_);
    CHECK(left->left->op == term_operation_t::variable);
    CHECK(left->left->variable == "m");
    CHECK(left->right->op == term_operation_t::complement);
    CHECK(left->right->left->op == term_operation_t::variable);
    CHECK(left->right->left->variable == "x42");

    auto right = static_cast<NTerm*>(ast->right);
    CHECK(right->op == term_operation_t::intersaction); // m * (-mx + -42x)
    CHECK(right->left->op == term_operation_t::variable);
    CHECK(right->left->variable == "m");
    CHECK(right->right->op == term_operation_t::union_); // (-mx + -42x)
    CHECK(right->right->left->op == term_operation_t::complement); // -mx
    CHECK(right->right->left->left->op == term_operation_t::variable);
    CHECK(right->right->left->left->variable == "mx");
    CHECK(right->right->right->op == term_operation_t::complement); // -42x
    CHECK(right->right->right->left->op == term_operation_t::variable);
    CHECK(right->right->right->left->variable == "42x");
}

TEST_CASE("atomic measured less eq", "[AST_structure]")
{
    auto ast = parse_from_input_string("<=m(m,m)", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::measured_less_eq);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::variable);
    CHECK(left->variable == "m");
    auto right = static_cast<NTerm*>(ast->right);
    CHECK(right->op == term_operation_t::variable);
    CHECK(right->variable == "m");
}

TEST_CASE("atomic measured less eq with complex terms", "[AST_structure]")
{
    auto ast = parse_from_input_string("<=m(m + -x42, m * (-mx + -42x))", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::measured_less_eq);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::union_);
    CHECK(left->left->op == term_operation_t::variable);
    CHECK(left->left->variable == "m");
    CHECK(left->right->op == term_operation_t::complement);
    CHECK(left->right->left->op == term_operation_t::variable);
    CHECK(left->right->left->variable == "x42");

    auto right = static_cast<NTerm*>(ast->right);
    CHECK(right->op == term_operation_t::intersaction); // m * (-mx + -42x)
    CHECK(right->left->op == term_operation_t::variable);
    CHECK(right->left->variable == "m");
    CHECK(right->right->op == term_operation_t::union_); // (-mx + -42x)
    CHECK(right->right->left->op == term_operation_t::complement); // -mx
    CHECK(right->right->left->left->op == term_operation_t::variable);
    CHECK(right->right->left->left->variable == "mx");
    CHECK(right->right->right->op == term_operation_t::complement); // -42x
    CHECK(right->right->right->left->op == term_operation_t::variable);
    CHECK(right->right->right->left->variable == "42x");
}

TEST_CASE("atomic eq zero", "[AST_structure]")
{
    auto ast = parse_from_input_string("ax = 0", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::eq_zero);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::variable);
    CHECK(left->variable == "ax");
}

TEST_CASE("atomic eq zero with complex term", "[AST_structure]")
{
    auto ast = parse_from_input_string("(ax + bx) * -cz = 0", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::eq_zero);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::intersaction);
    CHECK(left->left->op == term_operation_t::union_);
    CHECK(left->left->left->op == term_operation_t::variable);
    CHECK(left->left->left->variable == "ax");
    CHECK(left->left->right->op == term_operation_t::variable);
    CHECK(left->left->right->variable == "bx");
    CHECK(left->right->op == term_operation_t::complement);
    CHECK(left->right->left->op == term_operation_t::variable);
    CHECK(left->right->left->variable == "cz");
}

TEST_CASE("atomic formula with term constant 1", "[AST_structure]")
{
    auto ast = parse_from_input_string("0 = 0", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::eq_zero);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::constant_false);
}

TEST_CASE("atomic formula with term constant 2", "[AST_structure]")
{
    auto ast = parse_from_input_string("1 = 0", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::eq_zero);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::constant_true);
}

TEST_CASE("atomic formula with term constant 3", "[AST_structure]")
{
    auto ast = parse_from_input_string("C(a, (0 + -1) * a)", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::contact);
    auto left = static_cast<NTerm*>(ast->left);
    CHECK(left->op == term_operation_t::variable);
    CHECK(left->variable == "a");
    auto right = static_cast<NTerm*>(ast->right);
    CHECK(right->op == term_operation_t::intersaction); // (0 + -1) * a
    CHECK(right->left->op == term_operation_t::union_); // (0 + -1)
    CHECK(right->left->left->op == term_operation_t::constant_false);
    CHECK(right->left->right->op == term_operation_t::complement);
    CHECK(right->left->right->left->op == term_operation_t::constant_true);
    CHECK(right->right->op == term_operation_t::variable); // a
    CHECK(right->right->variable == "a");
}

TEST_CASE("complex formulas", "[AST_structure]")
{
    auto ast = parse_from_input_string("C(a, (0 + -1) * a) | (ax + bx) * -cz = 0", default_error_info_obj);
    CHECK(ast.get() != nullptr);
    CHECK(ast->op == formula_operation_t::disjunction);
    auto left = static_cast<NFormula*>(ast->left); // C(a, (0 + -1) * a)
    CHECK(left->op == formula_operation_t::contact); // a
    auto left_left = static_cast<NTerm*>(left->left);
    CHECK(left_left->op == term_operation_t::variable);
    CHECK(left_left->variable == "a");
    auto left_right = static_cast<NTerm*>(left->right);
    CHECK(left_right->op == term_operation_t::intersaction); // (0 + -1) * a
    CHECK(left_right->left->op == term_operation_t::union_); // (0 + -1)
    CHECK(left_right->left->left->op == term_operation_t::constant_false);
    CHECK(left_right->left->right->op == term_operation_t::complement);
    CHECK(left_right->left->right->left->op == term_operation_t::constant_true);
    CHECK(left_right->right->op == term_operation_t::variable); // a
    CHECK(left_right->right->variable == "a");

    auto right = static_cast<NFormula*>(ast->right); // (ax + bx) * -cz = 0
    CHECK(right->op == formula_operation_t::eq_zero);
    auto right_left = static_cast<NTerm*>(right->left);
    CHECK(right_left->op == term_operation_t::intersaction);
    CHECK(right_left->left->op == term_operation_t::union_);
    CHECK(right_left->left->left->op == term_operation_t::variable);
    CHECK(right_left->left->left->variable == "ax");
    CHECK(right_left->left->right->op == term_operation_t::variable);
    CHECK(right_left->left->right->variable == "bx");
    CHECK(right_left->right->op == term_operation_t::complement);
    CHECK(right_left->right->left->op == term_operation_t::variable);
    CHECK(right_left->right->left->variable == "cz");
}
