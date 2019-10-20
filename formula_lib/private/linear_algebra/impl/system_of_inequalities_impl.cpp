#include "system_of_inequalities_impl.h"

namespace
{
auto get_kiwi_operation(system_of_inequalities_impl::inequality_operation op) -> kiwi::RelationalOperator
{
    switch(op)
    {
        case system_of_inequalities_impl::inequality_operation::LE:
            return kiwi::RelationalOperator::OP_LE;
        default:
            assert(false);
            return kiwi::RelationalOperator::OP_LE;
    }
}
}

system_of_inequalities_impl::system_of_inequalities_impl(size_t number_of_variables)
    : number_of_variables_(number_of_variables)
    , is_solvable_(true)
    , variables_(number_of_variables_)
{
}

auto system_of_inequalities_impl::add_constraint(const variables_set& lhs, const variables_set& rhs,
                                                 inequality_operation op) -> bool
{
    if(!is_solvable())
    {
        return false;
    }

    // remove all variables which are in the lhs and rhs because they does not bring any value, we
    const auto common_variables = lhs & rhs;
    const auto common_variables_inverted = ~common_variables;
    const auto lhs_without_common_variables = lhs & common_variables_inverted;
    const auto rhs_without_common_variables = rhs & common_variables_inverted;

    auto construct_expression = [&](const variables_set& variables) -> kiwi::Expression {
        std::vector<kiwi::Term> terms;
        terms.reserve(variables.count());
        auto variable_id = variables.find_first();
        while(variable_id != variables_set::npos)
        {
            terms.emplace_back(variables_[variable_id]);
            variable_id = variables.find_next(variable_id);
        }
        return kiwi::Expression(terms);
    };

    kiwi::Expression lhs_expr = construct_expression(lhs_without_common_variables);
    kiwi::Expression rhs_expr = construct_expression(rhs_without_common_variables);
    kiwi::Constraint constraint(lhs_expr - rhs_expr, get_kiwi_operation(op));
    try
    {
        solver_.addConstraint(constraint);
    }
    catch(kiwi::DuplicateConstraint&)
    {
        // OK. the constraint is already added, NOOP
    }
    catch(kiwi::UnsatisfiableConstraint&)
    {
        is_solvable_ = false;
    }

    return is_solvable_;
}

auto system_of_inequalities_impl::is_solvable() const -> bool
{
    return is_solvable_;
}

auto system_of_inequalities_impl::get_variables_values() -> std::vector<double>
{
    if(!is_solvable())
    {
        return {};
    }

    solver_.updateVariables();

    std::vector<double> variables_values;
    variables_values.reserve(number_of_variables_);

    for(const auto& variable : variables_)
    {
        variables_values.push_back(variable.value());
    }

    return variables_values;
}

void system_of_inequalities_impl::clear()
{
    is_solvable_ = true;

    variables_.clear();
    variables_.resize(number_of_variables_); // TODO: maybe we can 'reset' the variables in another way?

    solver_.reset();
}
