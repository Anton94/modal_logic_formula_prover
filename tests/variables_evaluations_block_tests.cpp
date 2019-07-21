#include "catch/catch.hpp"

#include "library.h"
#include "variables_evaluations_block.h"

TEST_CASE("evaluation block of empty mask of variables", "[variables_evaluations_block]")
{
    variables_mask_t variables;
    variables_evaluations_blok var_ev_block(variables);

    CHECK(var_ev_block.get_variables() == variables);
    CHECK(var_ev_block.get_set_variables_ids().empty());
    CHECK(var_ev_block.get_evaluations().empty());
    CHECK(!var_ev_block.generate_next_evaluation());
}

TEST_CASE("evaluation block of mask with no set variables", "[variables_evaluations_block]")
{
    variables_mask_t variables(42, false);
    variables_evaluations_blok var_ev_block(variables);

    CHECK(var_ev_block.get_variables() == variables);
    CHECK(var_ev_block.get_set_variables_ids().empty());
    CHECK(var_ev_block.get_evaluations().size() == variables.size());
    CHECK(var_ev_block.get_evaluations().count() == 0);
    CHECK(!var_ev_block.generate_next_evaluation());

    // Checks that the evaluation is not broken after the failed generation
    CHECK(var_ev_block.get_evaluations().size() == variables.size());
    CHECK(var_ev_block.get_evaluations().count() == 0);
}

TEST_CASE("evaluation block of mask with one set variable", "[variables_evaluations_block]")
{
    std::vector<int> variables_sizes = { 1, 2, 3, 8, 42 };
    for (const auto num_of_vars : variables_sizes)
    {
        info() << "Variables size: " << num_of_vars;
        for (auto i = 0; i < num_of_vars; ++i)
        {
            variables_mask_t variables(num_of_vars, false);
            const variable_id_t set_var_id = i;
            info() << "Set variable is at: " << set_var_id;
            variables.set(set_var_id);
            variables_evaluations_blok var_ev_block(variables);

            CHECK(var_ev_block.get_variables() == variables);
            CHECK(var_ev_block.get_set_variables_ids() == variables_evaluations_blok::set_variables_ids_t{ set_var_id });
            CHECK(var_ev_block.get_evaluations().size() == variables.size());
            CHECK(var_ev_block.get_evaluations().count() == 0);
            CHECK(var_ev_block.generate_next_evaluation());

            CHECK(var_ev_block.get_evaluations().count() == 1);
            CHECK(var_ev_block.get_evaluations()[set_var_id]);
            CHECK(!var_ev_block.generate_next_evaluation());

            // Checks that the evaluation is not broken after the failed generation
            CHECK(var_ev_block.get_evaluations().count() == 1);
            CHECK(var_ev_block.get_evaluations()[set_var_id]);
        }
    }
}

TEST_CASE("evaluation block of mask with two set variables", "[variables_evaluations_block]")
{
    std::vector<int> variables_sizes = { 2, 3, 4, 8, 16, 42 };
    for (const auto num_of_vars : variables_sizes)
    {
        info() << "Variables size: " << num_of_vars;

        for (auto i = 0; i < num_of_vars; ++i)
        {
            for (auto j = i + 1; j < num_of_vars; ++j)
            {
                variables_mask_t variables(num_of_vars, false);
                const variable_id_t first_var_id = i;
                const variable_id_t second_var_id = j;
                info() << "Set variables are at: " << first_var_id << " and " << second_var_id;

                variables.set(first_var_id);
                variables.set(second_var_id);
                variables_evaluations_blok var_ev_block(variables);

                CHECK(var_ev_block.get_variables() == variables);
                CHECK(var_ev_block.get_set_variables_ids() == variables_evaluations_blok::set_variables_ids_t{ second_var_id, first_var_id });
                CHECK(var_ev_block.get_evaluations().size() == variables.size());

                // ..0..0..
                CHECK(var_ev_block.get_evaluations().count() == 0);
                CHECK(!var_ev_block.get_evaluations()[first_var_id]);
                CHECK(!var_ev_block.get_evaluations()[second_var_id]);
                CHECK(var_ev_block.generate_next_evaluation());

                // ..0..1..
                CHECK(var_ev_block.get_evaluations().count() == 1);
                CHECK(!var_ev_block.get_evaluations()[first_var_id]);
                CHECK(var_ev_block.get_evaluations()[second_var_id]);
                CHECK(var_ev_block.generate_next_evaluation());

                // ..1..0..
                CHECK(var_ev_block.get_evaluations().count() == 1);
                CHECK(var_ev_block.get_evaluations()[first_var_id]);
                CHECK(!var_ev_block.get_evaluations()[second_var_id]);
                CHECK(var_ev_block.generate_next_evaluation());

                // ..1..1..
                CHECK(var_ev_block.get_evaluations().count() == 2);
                CHECK(var_ev_block.get_evaluations()[first_var_id]);
                CHECK(var_ev_block.get_evaluations()[second_var_id]);
                CHECK(!var_ev_block.generate_next_evaluation());

                // Checks that the evaluation is not broken after the failed generation
                CHECK(var_ev_block.get_evaluations().count() == 2);
                CHECK(var_ev_block.get_evaluations()[first_var_id]);
                CHECK(var_ev_block.get_evaluations()[second_var_id]);
            }
        }
    }
}

TEST_CASE("evaluation block of mask with three set variables", "[variables_evaluations_block]")
{
    std::vector<int> variables_sizes = { 3, 4, 5, 10, 17 };
    for (const auto num_of_vars : variables_sizes)
    {
        info() << "Variables size: " << num_of_vars;
        for (auto i = 0; i < num_of_vars; ++i)
        {
            for (auto j = i + 1; j < num_of_vars; ++j)
            {
                for (auto k = j + 1; k < num_of_vars; ++k)
                {
                    variables_mask_t variables(num_of_vars, false);
                    const variable_id_t first_var_id = i;
                    const variable_id_t second_var_id = j;
                    const variable_id_t third_var_id = k;
                    info() << "Set variables are at: " << first_var_id << ", " << second_var_id << " and " << third_var_id;

                    variables.set(first_var_id);
                    variables.set(second_var_id);
                    variables.set(third_var_id);
                    variables_evaluations_blok var_ev_block(variables);

                    CHECK(var_ev_block.get_variables() == variables);
                    CHECK(var_ev_block.get_set_variables_ids() == variables_evaluations_blok::set_variables_ids_t{ third_var_id, second_var_id, first_var_id });
                    CHECK(var_ev_block.get_evaluations().size() == variables.size());

                    // ..0..0..0..
                    CHECK(var_ev_block.get_evaluations().count() == 0);
                    CHECK(!var_ev_block.get_evaluations()[first_var_id]);
                    CHECK(!var_ev_block.get_evaluations()[second_var_id]);
                    CHECK(!var_ev_block.get_evaluations()[third_var_id]);
                    CHECK(var_ev_block.generate_next_evaluation());

                    // ..0..0..1..
                    CHECK(var_ev_block.get_evaluations().count() == 1);
                    CHECK(!var_ev_block.get_evaluations()[first_var_id]);
                    CHECK(!var_ev_block.get_evaluations()[second_var_id]);
                    CHECK(var_ev_block.get_evaluations()[third_var_id]);
                    CHECK(var_ev_block.generate_next_evaluation());

                    // ..0..1..0..
                    CHECK(var_ev_block.get_evaluations().count() == 1);
                    CHECK(!var_ev_block.get_evaluations()[first_var_id]);
                    CHECK(var_ev_block.get_evaluations()[second_var_id]);
                    CHECK(!var_ev_block.get_evaluations()[third_var_id]);
                    CHECK(var_ev_block.generate_next_evaluation());

                    // ..0..1..1..
                    CHECK(var_ev_block.get_evaluations().count() == 2);
                    CHECK(!var_ev_block.get_evaluations()[first_var_id]);
                    CHECK(var_ev_block.get_evaluations()[second_var_id]);
                    CHECK(var_ev_block.get_evaluations()[third_var_id]);
                    CHECK(var_ev_block.generate_next_evaluation());

                    // ..1..0..0..
                    CHECK(var_ev_block.get_evaluations().count() == 1);
                    CHECK(var_ev_block.get_evaluations()[first_var_id]);
                    CHECK(!var_ev_block.get_evaluations()[second_var_id]);
                    CHECK(!var_ev_block.get_evaluations()[third_var_id]);
                    CHECK(var_ev_block.generate_next_evaluation());

                    // ..1..0..1..
                    CHECK(var_ev_block.get_evaluations().count() == 2);
                    CHECK(var_ev_block.get_evaluations()[first_var_id]);
                    CHECK(!var_ev_block.get_evaluations()[second_var_id]);
                    CHECK(var_ev_block.get_evaluations()[third_var_id]);
                    CHECK(var_ev_block.generate_next_evaluation());

                    // ..1..1..0..
                    CHECK(var_ev_block.get_evaluations().count() == 2);
                    CHECK(var_ev_block.get_evaluations()[first_var_id]);
                    CHECK(var_ev_block.get_evaluations()[second_var_id]);
                    CHECK(!var_ev_block.get_evaluations()[third_var_id]);
                    CHECK(var_ev_block.generate_next_evaluation());

                    // ..1..1..1..
                    CHECK(var_ev_block.get_evaluations().count() == 3);
                    CHECK(var_ev_block.get_evaluations()[first_var_id]);
                    CHECK(var_ev_block.get_evaluations()[second_var_id]);
                    CHECK(var_ev_block.get_evaluations()[third_var_id]);
                    CHECK(!var_ev_block.generate_next_evaluation());

                    // Checks that the evaluation is not broken after the failed generation
                    CHECK(var_ev_block.get_evaluations().count() == 3);
                    CHECK(var_ev_block.get_evaluations()[first_var_id]);
                    CHECK(var_ev_block.get_evaluations()[second_var_id]);
                    CHECK(var_ev_block.get_evaluations()[third_var_id]);
                }
            }
        }
    }
}
