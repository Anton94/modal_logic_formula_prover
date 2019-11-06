#include "catch/catch.hpp"

#include "library.h"
#include "variables_evaluations_block.h"
#include "variables_evaluations_block_stack.h"

namespace
{

void check_evaluation_blocks(const std::vector<variables_evaluations_block>& original_blocks)
{
    assert(!original_blocks.empty());
    const auto variables_size = original_blocks.front().get_variables_A().size();
    variables_evaluations_block_stack stack(variables_size);

    auto control_blocks = original_blocks;

    auto get_control_evaluations = [&]() {
        variables_evaluations_t control_evaluations(variables_size, false);
        for(const auto& block : control_blocks)
        {
            control_evaluations |= block.get_evaluations();
        }
        return control_evaluations;
    };

    auto get_control_variables = [&]() {
        variables_mask_t control_variables(variables_size, false);
        for(const auto& block : control_blocks)
        {
            control_variables |= block.get_variables_A();
        }
        return control_variables;
    };

    auto print_control_info = [&]() {
        trace() << "Running test with control blocks(bottom to top and combined):";
        for(const auto& block : control_blocks)
        {
            trace() << "\t" << block.get_variables_A() << "\n\t" << block.get_evaluations()
                    << "\n\t----------------------------------------------------";
        }
        trace() << "\tCombined:\n\t" << get_control_variables() << "\n\t" << get_control_evaluations();
    };

    for(const auto& block : original_blocks)
    {
        stack.push(block);
    }

    do
    {
        print_control_info();

        auto& control_top = control_blocks.back();

        CHECK(stack.top() == control_top);

        CHECK(get_control_evaluations() == stack.get_combined_evaluations());
        CHECK(get_control_variables() == stack.get_combined_variables());

        const auto has_generate_new_top_eval = stack.generate_evaluation();
        CHECK(control_top.generate_next_evaluation_over_A() == has_generate_new_top_eval);

        if(!has_generate_new_top_eval)
        {
            CHECK(stack.top() == control_top);

            CHECK(get_control_evaluations() == stack.get_combined_evaluations());
            CHECK(get_control_variables() == stack.get_combined_variables());

            control_blocks.pop_back();
            stack.pop();
        }
    } while(!stack.empty());
    CHECK(control_blocks.empty());

    CHECK(!stack.get_combined_variables().count());
    CHECK(!stack.get_combined_evaluations().count());

    CHECK(stack.get_combined_variables().size() == variables_size);
    CHECK(stack.get_combined_evaluations().size() == variables_size);
}
}

TEST_CASE("empty", "[variables_evaluations_block_stack]")
{
    variables_evaluations_block_stack stack(0);

    CHECK(stack.empty());
    CHECK(stack.get_combined_variables().empty());
    CHECK(stack.get_combined_evaluations().empty());
    CHECK(!stack.generate_evaluation());
}

TEST_CASE("one block with no set variables", "[variables_evaluations_block_stack]")
{
    const size_t size = 42;
    variables_evaluations_block b(variables_mask_t(size, false));
    variables_evaluations_block_stack stack(size);

    auto check_invariants = [&]() {
        CHECK(!stack.empty());
        CHECK(!stack.get_combined_variables().empty());
        CHECK(!stack.get_combined_evaluations().empty());

        CHECK(!stack.get_combined_variables().count());
        CHECK(!stack.get_combined_evaluations().count());

        CHECK(stack.get_combined_variables().size() == size);
        CHECK(stack.get_combined_evaluations().size() == size);

        CHECK(!stack.generate_evaluation());
    };

    trace() << "By copying";
    stack.push(b);
    check_invariants();

    stack.pop();

    trace() << "By moving";
    stack.push(std::move(b));
    check_invariants();
}

TEST_CASE("push block with one set variable and generated evaluation", "[variables_evaluations_block_stack]")
{
    const size_t size = 42;
    variables_mask_t varialbes_mask(size, false);
    varialbes_mask.set(14);

    variables_evaluations_block b(varialbes_mask);
    variables_evaluations_block_stack stack(size);

    b.generate_next_evaluation_over_A();
    stack.push(b);

    CHECK(stack.get_combined_variables() == b.get_variables_A());
    CHECK(stack.get_combined_evaluations() == b.get_evaluations());

    CHECK(!stack.generate_evaluation());
    CHECK(!b.generate_next_evaluation_over_A());
}

TEST_CASE("one block with set one variable", "[variables_evaluations_block_stack]")
{
    std::vector<size_t> variables_sizes = {1, 2, 3, 8, 42};
    for(const auto num_of_vars : variables_sizes)
    {
        info() << "Variables size: " << num_of_vars;
        for(size_t i = 0; i < num_of_vars; ++i)
        {
            variables_mask_t variables(num_of_vars, false);
            variables.set(i);
            check_evaluation_blocks({variables});
        }
    }
}

TEST_CASE("two blocks with one set variable in each", "[variables_evaluations_block_stack]")
{
    std::vector<size_t> variables_sizes = {2, 3, 8, 42};
    for(const auto num_of_vars : variables_sizes)
    {
        info() << "Variables size: " << num_of_vars;
        for(size_t i = 0; i < num_of_vars; ++i)
        {
            for(size_t j = i + 1; j < num_of_vars; ++j)
            {
                variables_mask_t variables_first(num_of_vars, false);
                variables_first.set(i);
                variables_mask_t variables_second(num_of_vars, false);
                variables_second.set(j);

                check_evaluation_blocks({variables_first, variables_second});

                check_evaluation_blocks({variables_second, variables_first});
            }
        }
    }
}

TEST_CASE("three blocks with one, two and three set variables", "[variables_evaluations_block_stack]")
{
    std::vector<size_t> variables_sizes = {6, 8, 12};
    for(const auto num_of_vars : variables_sizes)
    {
        info() << "Variables size: " << num_of_vars;
        for(size_t var_0 = 0; var_0 < num_of_vars; ++var_0)
        {
            for(size_t var_1 = var_0 + 1; var_1 < num_of_vars; ++var_1)
            {
                for(size_t var_2 = var_1 + 1; var_2 < num_of_vars; ++var_2)
                {
                    for(size_t var_3 = var_2 + 1; var_3 < num_of_vars; ++var_3)
                    {
                        for(size_t var_4 = var_3 + 1; var_4 < num_of_vars; ++var_4)
                        {
                            for(size_t var_5 = var_4 + 1; var_5 < num_of_vars; ++var_5)
                            {
                                variables_mask_t vars_first(num_of_vars, false);
                                vars_first.set(var_3);
                                variables_mask_t vars_second(num_of_vars, false);
                                vars_second.set(var_0);
                                vars_second.set(var_2);
                                variables_mask_t vars_third(num_of_vars, false);
                                vars_third.set(var_1);
                                vars_third.set(var_4);
                                vars_third.set(var_5);

                                CHECK(vars_first.count() == 1);
                                CHECK(vars_second.count() == 2);
                                CHECK(vars_third.count() == 3);

                                check_evaluation_blocks({vars_first, vars_second, vars_third}); // 1 2 3
                                check_evaluation_blocks({vars_first, vars_third, vars_second}); // 1 3 2
                                check_evaluation_blocks({vars_second, vars_first, vars_third}); // 2 1 3
                                check_evaluation_blocks({vars_second, vars_third, vars_first}); // 2 3 1
                                check_evaluation_blocks({vars_third, vars_first, vars_second}); // 3 1 2
                                check_evaluation_blocks({vars_third, vars_second, vars_first}); // 3 2 1
                            }
                        }
                    }
                }
            }
        }
    }
}

TEST_CASE("three blocks with one, two and three set variables and some generated evaluations",
          "[variables_evaluations_block_stack]")
{
    std::vector<size_t> variables_sizes = {6, 8, 12};
    for(const auto num_of_vars : variables_sizes)
    {
        info() << "Variables size: " << num_of_vars;
        for(size_t var_0 = 0; var_0 < num_of_vars; ++var_0)
        {
            for(size_t var_1 = var_0 + 1; var_1 < num_of_vars; ++var_1)
            {
                for(size_t var_2 = var_1 + 1; var_2 < num_of_vars; ++var_2)
                {
                    for(size_t var_3 = var_2 + 1; var_3 < num_of_vars; ++var_3)
                    {
                        for(size_t var_4 = var_3 + 1; var_4 < num_of_vars; ++var_4)
                        {
                            for(size_t var_5 = var_4 + 1; var_5 < num_of_vars; ++var_5)
                            {
                                variables_mask_t vars_first(num_of_vars, false);
                                vars_first.set(var_3);
                                variables_mask_t vars_second(num_of_vars, false);
                                vars_second.set(var_0);
                                vars_second.set(var_2);
                                variables_mask_t vars_third(num_of_vars, false);
                                vars_third.set(var_1);
                                vars_third.set(var_4);
                                vars_third.set(var_5);

                                variables_evaluations_block first(vars_first);
                                variables_evaluations_block second(vars_second);
                                variables_evaluations_block third(vars_third);
                                CHECK(second.generate_next_evaluation_over_A());
                                CHECK(second.get_evaluations().count() == 1);

                                CHECK(third.generate_next_evaluation_over_A());
                                CHECK(third.generate_next_evaluation_over_A());
                                CHECK(third.generate_next_evaluation_over_A());
                                CHECK(third.get_evaluations().count() == 2);

                                CHECK(first.get_variables_A().count() == 1);
                                CHECK(second.get_variables_A().count() == 2);
                                CHECK(third.get_variables_A().count() == 3);

                                check_evaluation_blocks({first, second, third}); // 1 2 3
                                check_evaluation_blocks({first, third, second}); // 1 3 2
                                check_evaluation_blocks({second, first, third}); // 2 1 3
                                check_evaluation_blocks({second, third, first}); // 2 3 1
                                check_evaluation_blocks({third, first, second}); // 3 1 2
                                check_evaluation_blocks({third, second, first}); // 3 2 1
                            }
                        }
                    }
                }
            }
        }
    }
}
