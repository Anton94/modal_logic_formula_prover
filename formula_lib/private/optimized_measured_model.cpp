#include "optimized_measured_model.h"
#include "formula.h"
#include "formula_mgr.h"
#include "term.h"
#include "thread_termiator.h"
#include "utils.h"

#include <algorithm>
#include <cassert>

optimized_measured_model::optimized_measured_model()
    : model()
    , additional_point_initial_value_(variables_mask_t(0))
    , system_(0)
{
}

optimized_measured_model::~optimized_measured_model() = default;

auto optimized_measured_model::create(const formulas_t& contacts_T, const formulas_t& contacts_F,
                                      const terms_t& zero_terms_T, const terms_t& zero_terms_F,
                                      const formulas_t& measured_less_eq_T,
                                      const formulas_t& measured_less_eq_F,
                                      const variables_mask_t& used_variables, const formula_mgr* mgr) -> bool
{
    clear();

    if(!model::create(contacts_T, contacts_F, zero_terms_T, zero_terms_F, measured_less_eq_T,
                      measured_less_eq_F, used_variables, mgr))
    {
        trace() << "Unable to construct even a model without the measured atomics";
        return false;
    }

    points_count_without_additional_ = points_.size();
    trace() << "Constructed a satisfiable model:\n" << *this;

    measured_less_eq_T_ = measured_less_eq_T;
    measured_less_eq_F_ = measured_less_eq_F;

    // One addition point per atomic measured formula is needed in order the system to be able to find a
    // solution.
    const auto additional_points_for_measured_atomics = measured_less_eq_T.size() + measured_less_eq_F.size();
    if(additional_points_for_measured_atomics > 0)
    {
        info() << "Adding additional " << points_count_without_additional_
               << " point(s) because of the <=m/~<=m atomics. "
                  "We need a potential one model point for each side of each inequality in the system.";

        assert(!points_.empty());
        construct_additional_points(contacts_F, zero_terms_T, additional_points_for_measured_atomics);
    }

    create_contact_relations_first_2k_in_contact(points_.size(), contacts_T.size());
    calculate_the_model_evaluation_of_each_variable();

    system_ = system_of_inequalities(points_.size());
    while(!has_solvable_system_of_inequalities())
    {
        TERMINATE_IF_NEEDED();
        if(!generate_next_additional_points_combination(contacts_F, zero_terms_T))
        {
            trace() << "Unable to generate a new combination of binary var. evaluations for the additional "
                       "model points to satisfy the system.";
            return false;
        }
    }

    return true;
}

auto optimized_measured_model::generate_next_additional_points_combination(const formulas_t& contacts_F,
                                                                           const terms_t& zero_terms_T)
    -> bool
{
    // Generate new combination of points only for the additional one.
    // Note that they doesn't effect the satisfiability of the model, they are needed only to add some
    // additional variables in the
    // inequalities to be able to make a solvable system.

    // TODO: maybe cache the valid evaluations per term
    // simulation of +1 operation from left to right but not just 0/1 bits but all combinations per element
    for(auto i = points_count_without_additional_, points_size = points_.size(); i < points_size; ++i)
    {
        auto& point = points_[i];
        while(point.generate_next_evaluation_over_A())
        {
            if(are_zero_terms_T_satisfied(zero_terms_T, point) &&
               is_contacts_F_reflexive_rule_satisfied(contacts_F, point))
            {
                // TODO: this maybe can be done smarter and not just whole new calculation
                calculate_the_model_evaluation_of_each_variable();
                return true;
            }
        }

        point = additional_point_initial_value_; // reset the point's evaluation
    }
    return false;
}

auto optimized_measured_model::has_solvable_system_of_inequalities() -> bool
{
    // For each <=m(a,b) calculate v(a) and v(b), then we will create
    // an inequality of the following type: SUM_I Xi <= SUM_J Xj, where I is v(a) and J is v(b).
    // For each ~<=m(a,b) calculate v(a) and v(b), then we will create
    // an inequality of the following type: SUM_I Xi > SUM_J Xj, where I is v(a) and J is v(b).
    // Each inequality is a row in the system of inequalities. If this system has a solution, then we are
    // good.

    const auto points_size = points_.size();
    system_.clear();

    for(const auto& m : measured_less_eq_T_)
    {
        const auto v_a = m->get_left_child_term()->evaluate(variable_evaluations_, points_size);
        const auto v_b = m->get_right_child_term()->evaluate(variable_evaluations_, points_size);
        if(!system_.add_constraint(v_a, v_b, system_of_inequalities::inequality_operation::LE))
        {
            verbose() << "Unable to find a solution for the system when adding the constraint for " << *m
                      << "\n"
                      << *static_cast<imodel* const>(
                             this); // TODO: why not able to call directly *this??? why unresolved ???
            return false;
        }
    }

    for(const auto& m : measured_less_eq_F_)
    {
        const auto v_a = m->get_left_child_term()->evaluate(variable_evaluations_, points_size);
        const auto v_b = m->get_right_child_term()->evaluate(variable_evaluations_, points_size);
        if(!system_.add_constraint(v_a, v_b, system_of_inequalities::inequality_operation::G))
        {
            std::stringstream out;
            print_system(out);
            verbose() << "Unable to find a solution for the system when adding the constraint for ~" << *m
                      << "\n"
                      << *static_cast<imodel* const>(this);
            return false;
        }
    }

    info() << "Found a solution for the system:\n" << *static_cast<imodel* const>(this);
    return true;
}

void optimized_measured_model::construct_additional_points(const formulas_t& contacts_F,
                                                           const terms_t& zero_terms_T,
                                                           size_t points_count_to_construct)
{
    if(points_count_to_construct <= 0)
    {
        return;
    }

    assert(!points_.empty());
    // add a point to the back of @points_
    const auto is_constructed =
        construct_point(contacts_F, zero_terms_T); // TODO: maybe change the method to return an evaluation or
                                                   // something, might be more clear that way
    assert(is_constructed); // always should be able to construct such point, because we can duplicate the
                            // evaluation of some existing point
    (void)is_constructed;

    additional_point_initial_value_ = points_.back();

    if(points_count_to_construct > 1)
    {
        // replicate it to add a total of @points_count_to_construct points.
        points_.insert(points_.end(), points_count_to_construct - 1, additional_point_initial_value_);
    }
}

void optimized_measured_model::clear()
{
    additional_point_initial_value_ = variables_evaluations_block(variables_mask_t(0));
    points_count_without_additional_ = 0;
    measured_less_eq_T_.clear();
    measured_less_eq_F_.clear();
    system_.clear();

    model::clear();
}

auto optimized_measured_model::print_system_sum_variables(std::ostream& out,
                                                          const model_points_set_t& variables) const
    -> std::ostream&
{
    // iterate only set bits(1s)
    auto var_id = variables.find_first();
    bool has_printed_first = false;
    if(var_id == variables_evaluations_t::npos)
    {
        out << "0";
        return out;
    }

    while(var_id != variables_evaluations_t::npos)
    {
        if(has_printed_first)
        {
            out << " + ";
        }
        out << "X" << var_id;
        has_printed_first = true;
        var_id = variables.find_next(var_id);
    }
    return out;
}

auto optimized_measured_model::print_system(std::ostream& out) const -> std::ostream&
{
    const auto points_size = points_.size();
    out << "System:\n";
    for(const auto& m : measured_less_eq_T_) // <=m(a,b)
    {
        const auto v_a = m->get_left_child_term()->evaluate(variable_evaluations_, points_size);
        const auto v_b = m->get_right_child_term()->evaluate(variable_evaluations_, points_size);

        out << "| ";
        print_system_sum_variables(out, v_a);
        out << " <= ";
        print_system_sum_variables(out, v_b);
        out << " # " << *m << "\n";
    }
    for(const auto& m : measured_less_eq_F_) // ~<=m(a,b)
    {
        const auto v_a = m->get_left_child_term()->evaluate(variable_evaluations_, points_size);
        const auto v_b = m->get_right_child_term()->evaluate(variable_evaluations_, points_size);

        out << "| ";
        print_system_sum_variables(out, v_a);
        out << " > ";
        print_system_sum_variables(out, v_b);
        out << " # ~" << *m << "\n";
    }
    return out;
}

auto optimized_measured_model::print(std::ostream& out) const -> std::ostream&
{
    out << "Model points: \n";
    const auto points_size = points_.size();
    for(size_t i = 0; i < points_size; ++i)
    {
        if(i == points_count_without_additional_)
        {
            out << "Additional model points for the measured atomics: \n";
        }
        out << std::to_string(i) << " : ";
        mgr_->print(out, points_[i]);
        out << "\n";
    }

    if(measured_less_eq_T_.empty() && measured_less_eq_F_.empty())
    {
        return out;
    }

    out << "Measured  : " << measured_less_eq_T_ << "\n";
    out << "Measured ~: " << measured_less_eq_F_ << "\n";

    print_system(out);

    if(system_.is_solvable())
    {
        out << "Solution:\n";
        auto variables_values = system_.get_variables_values();
        assert(variables_values.size() == points_size);
        for(size_t i = 0; i < points_size; ++i)
        {
            out << "X" << i << " = " << variables_values[i] << "\n";
        }
    }
    else
    {
        out << "No solution!\n";
    }

    return out;
}
