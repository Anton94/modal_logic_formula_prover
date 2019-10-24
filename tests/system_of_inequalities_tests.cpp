#include "catch/catch.hpp"

#include "library.h"
#include "linear_algebra/system_of_inequalities.h"

#include <iostream>

using variables_set = system_of_inequalities::variables_set;

namespace
{

struct inequality
{
    variables_set lhs;
    variables_set rhs;
    system_of_inequalities::inequality_operation op = system_of_inequalities::inequality_operation::LE;
};


void check_inequalities(size_t number_of_variables, const std::vector<inequality>& inequalities,
                        bool is_last_inequality_solvable = true)
{
    CHECK(!inequalities.empty());

    system_of_inequalities system(number_of_variables);

    CHECK(system.is_solvable());

    for(size_t i = 0, count = inequalities.size() - 1; i < count; ++i)
    {
        const auto& inequality = inequalities[i];

        CHECK(system.add_constraint(inequality.lhs, inequality.rhs, inequality.op));
    }

    const auto& last_inequality = inequalities.back();
    CHECK(system.add_constraint(last_inequality.lhs, last_inequality.rhs, last_inequality.op) ==
          is_last_inequality_solvable);
    CHECK(system.is_solvable() == is_last_inequality_solvable);

    // TODO: check if the provided values satisfies the system (via system.get_variables_values())
    // check them after each added inequality
    const auto variables_values = system.get_variables_values();
    CHECK(variables_values.size() == number_of_variables);

    // TODO: remove the printing when the automatic variable's value checking is done
    for(size_t i = 0; i < number_of_variables; ++i)
    {
        std::cout << "X" << i << " : " << variables_values[i] << "\n";
    }
}
}

TEST_CASE("system_of_inequalities 1", "[system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 4;

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);

        i.rhs.set(0);
        i.rhs.set(1);
        i.rhs.set(2);
        i.rhs.set(3);
        // X0 + X1 + X2 + X3 <= 0
        inequalities.emplace_back(std::move(i));
    }

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);

        i.lhs.set(0);
        i.lhs.set(1);

        i.rhs.set(2);
        i.rhs.set(3);
        // X0 + X1 <= X2 + X3
        inequalities.emplace_back(std::move(i));
    }

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);

        i.rhs.set(0);
        i.rhs.set(1);

        i.lhs.set(2);
        i.lhs.set(3);
        // X2 + X3 <= X0 + X1
        inequalities.emplace_back(std::move(i));
    }
    check_inequalities(number_of_variables, inequalities);
}

TEST_CASE("system_of_inequalities 2", "[system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 4;

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(0);
        i.lhs.set(1);
        i.lhs.set(2);
        i.lhs.set(3);
        // X0 + X1 + X2 + X3 > 0
        inequalities.emplace_back(std::move(i));
    }

    check_inequalities(number_of_variables, inequalities);
}

TEST_CASE("system_of_inequalities 3", "[system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 4;

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(0);
        i.lhs.set(1);
        i.lhs.set(2);
        i.lhs.set(3);
        // X0 + X1 + X2 + X3 > 0
        inequalities.emplace_back(std::move(i));
    }

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);

        i.lhs.set(3);
        i.lhs.set(2);
        // X2 + X3 <= 0
        inequalities.emplace_back(std::move(i));
    }

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(0);
        // X0 > 0
        inequalities.emplace_back(std::move(i));
    }
    check_inequalities(number_of_variables, inequalities);
}
