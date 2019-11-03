#pragma once

#include "../system_of_inequalities.h" // for the types TODO: refactor?
#include "kiwi/kiwi.h"


class system_of_inequalities_impl
{
    /*
     * See system_of_inequalities.h for more info!
     */
public:
    system_of_inequalities_impl(size_t number_of_variables);

    using variables_set = system_of_inequalities::variables_set;
    using inequality_operation = system_of_inequalities::inequality_operation;

    auto add_constraint(const variables_set& lhs, const variables_set& rhs, inequality_operation op) -> bool;

    auto is_solvable() const -> bool;

    auto get_variables_values() -> std::vector<double>;

    void clear();

private:
    // Adds constraints of the type: Xi >= epsilon
    void add_constraints_for_positive_variables();
    void add_constraint_for_positive_variable(const kiwi::Variable& x);

    size_t number_of_variables_;
    bool is_solvable_;

    std::vector<kiwi::Variable> variables_;
    kiwi::Solver solver_;
};
