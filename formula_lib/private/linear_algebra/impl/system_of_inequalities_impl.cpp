#include "system_of_inequalities_impl.h"

#include <limits>

static const kiwi::Expression epsilon(std::numeric_limits<double>::epsilon());

system_of_inequalities_impl::system_of_inequalities_impl(size_t number_of_variables)
    : number_of_variables_(number_of_variables)
    , is_solvable_(true)
    , variables_(number_of_variables_)
{
    add_constraints_for_positive_variables();
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

    assert(op == inequality_operation::G || op == inequality_operation::LE);

    kiwi::Constraint constraint;
    if (op == inequality_operation::LE)
    {
        // X <= Y ---> X - Y <= 0
        constraint = { lhs_expr - rhs_expr, kiwi::OP_LE };
    }
    else
    {
        assert(op == inequality_operation::G);
        // create a new positive variable Z
        variables_.emplace_back();
        auto& z = variables_.back();
        add_constraint_for_positive_variable(z);
        // X > Y ---> X - Y > 0 ---#multiply by -1#---> Y - X < 0 ---> Y - X + Z <= 0
        constraint = { rhs_expr - lhs_expr + z, kiwi::OP_LE };
    }

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
    variables_values.resize(number_of_variables_);

    // skip the Zs because they are just for internal usage to simulate inequalities of type '>'
    for(size_t i = 0; i < number_of_variables_; ++i)
    {
        variables_values[i] = variables_[i].value();
    }

    return variables_values;
}

void system_of_inequalities_impl::clear()
{
    is_solvable_ = true;

    variables_.clear();
    variables_.resize(number_of_variables_); // TODO: maybe we can 'reset' the variables in another way?
    add_constraints_for_positive_variables();

    solver_.reset();
}

void system_of_inequalities_impl::add_constraints_for_positive_variables()
{
    for (const auto& variable : variables_)
    {
        add_constraint_for_positive_variable(variable);
    }
}

void system_of_inequalities_impl::add_constraint_for_positive_variable(const kiwi::Variable& x)
{
    try
    {
        solver_.addConstraint({ x - 0.00000001, kiwi::OP_GE });
    }
    catch (kiwi::DuplicateConstraint&)
    {
        assert(false && "This constraint should be unique, we do not have duplication of variables and they should be unique!");
    }
    catch (kiwi::UnsatisfiableConstraint&)
    {
        assert(false && "This constraint should be always satisfiable!");
    }
}