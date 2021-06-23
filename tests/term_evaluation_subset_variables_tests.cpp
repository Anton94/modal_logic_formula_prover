#include "catch/catch.hpp"

#include "library.h"
#include "term.h"
#include "variables_evaluations_block.h"

#include <utility>
#include <vector>

namespace
{

formula_mgr create_atomic_formula(const std::string& left, const std::string& right)
 {
    std::stringstream s;
    s << "C(" << left << "," << right << ")";
    formula_mgr f;
    CHECK(f.build(s.str(), formula_mgr::formula_refiners::none));

    return f;
}

void evaluate(const std::string& term_for_evaluation, const std::string& expected_term,
              const std::vector<std::pair<std::string, bool>>& evaluations)
{
    auto mgr = create_atomic_formula(term_for_evaluation, expected_term);
    auto t = mgr.get_internal_formula()->get_left_child_term();

    variables_mask_t variables_mask(t->get_variables().size(), 0);
    variables_evaluations_t evaluation_mask(t->get_variables().size(), 0);
    for(const auto& evaluation : evaluations)
    {
        const auto& variable_name = evaluation.first;
        const auto variable_evaluation = evaluation.second;
        const auto varialbe_id = mgr.get_variable(variable_name);
        assert(varialbe_id < t->get_variables().size());
        variables_mask.set(varialbe_id);
        evaluation_mask.set(varialbe_id, variable_evaluation);
    }

    variables_evaluations_block block(variables_mask);
    block.get_evaluations() = evaluation_mask;

    trace() << "Evaluating " << *t << " with: ";
    for(size_t i = 0, size = variables_mask.size(); i < size; ++i)
    {
        if(variables_mask[i])
        {
            trace() << "    " << mgr.get_variable(i) << " : " << evaluation_mask[i];
        }
    }

    auto res = t->evaluate(block, false);
    term* evaluated_term{nullptr};
    const auto is_res_term = res.is_term();
    if(is_res_term)
    {
        evaluated_term = res.release();
        trace() << "Result: " << *evaluated_term;
    }
    else
    {
        trace() << "Result: " << (res.is_constant_true() ? "constant true" : "constant false");
    }

    if(expected_term == "1")
    {
        CHECK(res.is_constant_true());
    }
    else if (expected_term == "0")
    {
        CHECK(res.is_constant_false());
    }
    else
    {
        if(is_res_term)
        {
            auto expected = mgr.get_internal_formula()->get_right_child_term();
            CHECK(*evaluated_term == *expected);
            delete evaluated_term;
        }
        else
        {
            CHECK((false && "Expected the evaluation to be a term but got a constant."));
        }
    }
}
}

TEST_CASE("1", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + c", "1", {{"a", true}, {"b", false}, {"c", true}});
}

TEST_CASE("2", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + c", "1", {{"a", true}, {"b", true}, {"c", false}});
}

TEST_CASE("3", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + c", "0", {{"a", true}, {"b", false}, {"c", false}});
}

TEST_CASE("4", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "1", {{"a", true}, {"b", false}, {"c", false}});
}

TEST_CASE("5", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "1", {{"a", true}, {"b", true}, {"c", false}});
}

TEST_CASE("6", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "0", {{"a", false}, {"b", true}, {"c", false}});
}

TEST_CASE("7", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "-c", {{"a", false}, {"b", false}});
}

TEST_CASE("8", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "-c", {{"a", true}, {"b", false}});
}

TEST_CASE("9", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "a", {{"b", true}, {"c", true}});
}

TEST_CASE("10", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "a", {{"b", true}});
}

TEST_CASE("11", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "a * b", {{"c", true}});
}

TEST_CASE("12", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "b + -b", {{"c", false}, {"a", true}});
}

TEST_CASE("13", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "-(b + c)", {{"a", false}});
}

TEST_CASE("14", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "b + -(b + c)", {{"a", true}});
}

TEST_CASE("15", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "-c", {{"b", false}});
}

TEST_CASE("16", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "0", {{"a", false}, {"b", true}});
}

TEST_CASE("17", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "1", {{"a", false}, {"b", false}, {"c", false}});
}

TEST_CASE("18", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "1", {{"a", true}, {"b", true}, {"c", true}});
}

TEST_CASE("19", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "0", {{"a", true}, {"b", false}, {"c", true}});
}

TEST_CASE("20", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "0", {{"a", false}, {"b", true}, {"c", true}});
}

TEST_CASE("21", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "0",
        {{"a", false}, {"b", false}, {"c", true}});
}

TEST_CASE("22", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "1", {{"b", false}, {"c", false}});
}

TEST_CASE("23", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "0", {{"b", false}, {"c", true}});
}

TEST_CASE("24", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "a", {{"b", true}, {"c", false}});
}

TEST_CASE("25", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "a", {{"b", true}, {"c", true}});
}

TEST_CASE("26", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "-b", {{"a", false}, {"c", false}});
}

TEST_CASE("27", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "0", {{"a", false}, {"c", true}});
}

TEST_CASE("28", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "b", {{"a", true}, {"c", true}});
}

TEST_CASE("29", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "(a * b) + -b", {{"c", false}});
}

TEST_CASE("30 without evaluation", "[term_evaluation_with_subset_variables]")
{
    evaluate("(a * b) + -(b + c)", "(a * b) + -(b + c)", {});
}
