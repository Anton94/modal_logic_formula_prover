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

    auto check_variable_values = [&](size_t verify_first_n_inequalities)
    {
        const auto variables_values = system.get_variables_values();
        CHECK(variables_values.size() == number_of_variables);

        for (size_t i = 0; i < variables_values.size(); ++i)
        {
            CHECK(variables_values[i] > 0);
            // std::cout << "X" << i << " = " << variables_values[i] << std::endl;
        }

        auto sum_of_variables = [&](const variables_set& variables)
        {
            double sum = 0;
            auto variable_id = variables.find_first();
            while(variable_id != variables_set::npos)
            {
                sum += variables_values[variable_id];
                variable_id = variables.find_next(variable_id);
            }
            return sum;
        };

        assert(verify_first_n_inequalities <= inequalities.size());
        for(size_t i = 0; i < verify_first_n_inequalities; ++i)
        {
            const auto& inequality = inequalities[i];
            double lhs_sum = sum_of_variables(inequality.lhs);
            double rhs_sum = sum_of_variables(inequality.rhs);
            if(inequality.op == system_of_inequalities::inequality_operation::G)
            {
                CHECK(lhs_sum > rhs_sum);
            }
            else
            {
                assert(inequality.op == system_of_inequalities::inequality_operation::LE);
                CHECK((lhs_sum < rhs_sum || lhs_sum == Approx(rhs_sum)));
            }
        }
    };

    for(size_t i = 0, count = inequalities.size() - 1; i < count; ++i)
    {
        const auto& inequality = inequalities[i];

        CHECK(system.add_constraint(inequality.lhs, inequality.rhs, inequality.op));
        CHECK(system.is_solvable());
        check_variable_values(i + 1);
    }

    const auto& last_inequality = inequalities.back();
    CHECK(system.add_constraint(last_inequality.lhs, last_inequality.rhs, last_inequality.op) == is_last_inequality_solvable);
    CHECK(system.is_solvable() == is_last_inequality_solvable);

    if(is_last_inequality_solvable)
    {
        check_variable_values(inequalities.size());
    }
}
}

TEST_CASE("system_of_inequalities 0", "[system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 4;

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(0);

        // X0 > 0
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);

        i.lhs.set(0);

        // X0 <= 0
        inequalities.emplace_back(std::move(i));
    }

    check_inequalities(number_of_variables, inequalities, false);
}

TEST_CASE("system_of_inequalities 1", "[system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 4;

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);

        i.lhs.set(0);
        i.lhs.set(1);
        i.lhs.set(2);
        i.lhs.set(3);

        // X0 + X1 + X2 + X3 <= 0
        inequalities.emplace_back(std::move(i));
    }

    check_inequalities(number_of_variables, inequalities, false);
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

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::LE;

        i.lhs.set(0);
        i.rhs.set(1);
        // X0 <= X1
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
        i.rhs.set(1);
        // X0 > X1
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::LE;

        i.lhs.set(0);
        i.rhs.set(1);
        // X0 <= X1
        inequalities.emplace_back(std::move(i));
    }
    check_inequalities(number_of_variables, inequalities, false);
}

TEST_CASE("system_of_inequalities 4", "[system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 4;

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(0);
        i.rhs.set(1);
        // X0 > X1
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(1);
        i.rhs.set(2);
        // X1 > X2
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::LE;

        i.lhs.set(0);
        i.rhs.set(2);
        // X0 <= X2
        inequalities.emplace_back(std::move(i));
    }
    check_inequalities(number_of_variables, inequalities, false);
}

TEST_CASE("system_of_inequalities 5", "[system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 4;

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
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(0);
        i.rhs.set(3);
        // X0 > X3
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(1);
        i.rhs.set(3);
        // X1 > X3
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(1);
        i.rhs.set(2);
        // X1 > X2
        inequalities.emplace_back(std::move(i));
    }
    check_inequalities(number_of_variables, inequalities, false);
}

TEST_CASE("system_of_inequalities 6", "[system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 5;

    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);

        i.lhs.set(0);

        i.rhs.set(1);
        i.rhs.set(2);
        i.rhs.set(3);
        i.rhs.set(4);
        // X0 <= X1 + X2 + X3 + X4
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(0);
        i.rhs.set(1);

        // X0 > X1
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(1);
        i.rhs.set(2);

        // X1 > X2
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(2);
        i.rhs.set(3);

        // X2 > X3
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(3);
        i.rhs.set(4);

        // X3 > X4
        inequalities.emplace_back(std::move(i));
    }
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(4);
        i.rhs.set(0);

        // X4 > X1
        inequalities.emplace_back(std::move(i));
    }
    check_inequalities(number_of_variables, inequalities, false);
}

TEST_CASE("system_of_inequalities 100 variables 100 inequalities of type Xk > 0", "[system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 100;

    for(size_t k = 0; k < 100; ++k)
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(k);
        // Xk > 0
        inequalities.emplace_back(std::move(i));
    }
    check_inequalities(number_of_variables, inequalities);
}

TEST_CASE("system_of_inequalities 101 variables 100 inequalities of type Xk > Xk+1", "[system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 101;

    for(size_t k = 0; k < 100; ++k)
    {
        inequality i;
        i.lhs = variables_set(number_of_variables);
        i.rhs = variables_set(number_of_variables);
        i.op = system_of_inequalities::inequality_operation::G;

        i.lhs.set(k);
        i.rhs.set(k + 1);
        // Xk > Xk+1
        inequalities.emplace_back(std::move(i));
    }
    check_inequalities(number_of_variables, inequalities);
}

TEST_CASE("system_of_inequalities 101 variables 100 inequalities of type X0 + .. + Xk > Xk+1 + ... + X100 and 100 inequalities of type Xk > Xk+1", "[!hide][system_of_inequalities]")
{
    std::vector<inequality> inequalities;
    size_t number_of_variables = 101;

    for(size_t k = 0; k < 100; ++k)
    {
        {
            inequality i;
            i.lhs = variables_set(number_of_variables);
            i.rhs = variables_set(number_of_variables);
            i.op = system_of_inequalities::inequality_operation::G;

            for(size_t j = 0; j <= k; ++j)
            {
                i.lhs.set(j);
            }
            for(size_t j = k + 1; j < number_of_variables; ++j)
            {
                i.rhs.set(j);
            }
            // X0 + .. + Xk > Xk+1 + ... + X100
            inequalities.emplace_back(std::move(i));
        }
        {
            inequality i;
            i.lhs = variables_set(number_of_variables);
            i.rhs = variables_set(number_of_variables);
            i.op = system_of_inequalities::inequality_operation::G;

            i.lhs.set(k);
            i.rhs.set(k + 1);
            // Xk > Xk+1
            inequalities.emplace_back(std::move(i));
        }
    }
    check_inequalities(number_of_variables, inequalities);
}
