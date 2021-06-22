#include "model.h"
#include "formula.h"
#include "formula_mgr.h"
#include "term.h"
#include "../include/thread_termiator.h"

#include <cassert>

auto model::create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
                   const terms_t& zero_terms_F, const formulas_t&, const formulas_t&, const variables_mask_t& used_variables,
                   const formula_mgr* mgr) -> bool
{
    clear();
    used_variables_ = used_variables;
    mgr_ = mgr;

    if(!construct_contact_model_points(contacts_T, contacts_F, zero_terms_T) ||
       !construct_non_zero_model_points(zero_terms_F, contacts_F, zero_terms_T))
    {
        return false;

    }

    if(points_.empty() && !construct_point(contacts_F, zero_terms_T))
    {
        return false;
    }

    calculate_the_model_evaluation_of_each_variable();
    create_contact_relations_first_2k_in_contact(points_.size(), contacts_T.size());
    return true;
}

auto model::get_model_points() const -> const points_t&
{
    return points_;
}

void model::clear()
{
    used_variables_.clear();
    points_.clear();

    imodel::clear();
}

auto model::construct_contact_model_points(const formulas_t& contacts_T, const formulas_t& contacts_F,
                                           const terms_t& zero_terms_T) -> bool
{
    for(const auto& c : contacts_T)
    {
        TERMINATE_IF_NEEDED();
        if(!construct_contact_points(c, contacts_F, zero_terms_T))
        {
            return false;
        }
    }

    return true;
}

auto model::construct_non_zero_model_points(const terms_t& zero_terms_F, const formulas_t& contacts_F,
                                            const terms_t& zero_terms_T) -> bool
{
    for(const auto& z : zero_terms_F)
    {
        TERMINATE_IF_NEEDED();
        // TODO: optimize by checking if there is already existing point with evaluation which evaluates the
        // term z to true
        variables_evaluations_block eval(variables_mask_t(0)); // it will be overridden if succeed
        if(!create_point_evaluation(z, eval, contacts_F, zero_terms_T))
        {
            return false;
        }
        points_.push_back(std::move(eval));
    }

    return true;
}

auto model::create_point_evaluation(const term* t, variables_evaluations_block& out_evaluation,
                                    const formulas_t& contacts_F, const terms_t& zero_terms_T) const -> bool
{
    out_evaluation = variables_evaluations_block(used_variables_);

    return does_point_evaluation_satisfies_basic_rules(t, out_evaluation, contacts_F, zero_terms_T) ||
           generate_next_point_evaluation(t, out_evaluation, contacts_F, zero_terms_T);
}

auto model::generate_next_point_evaluation(const term* t, variables_evaluations_block& out_evaluation,
                                           const formulas_t& contacts_F, const terms_t& zero_terms_T) const
    -> bool
{

    while(out_evaluation.generate_next_evaluation_over_A())
    {
        // TODO the generation can be done smarter, e.g. when the evaluation of 't' is false, generate new
        // variable evaluations just for the varaibles in @t.
        if(does_point_evaluation_satisfies_basic_rules(t, out_evaluation, contacts_F, zero_terms_T))
        {
            return true;
        }
    }
    return false;
}

auto model::construct_contact_points(const formula* c, const formulas_t& contacts_F,
                                     const terms_t& zero_terms_T) -> bool
{
    const auto left = c->get_left_child_term();
    const auto right = c->get_right_child_term();

    variables_evaluations_block left_eval(variables_mask_t(0)); // it will be overridden if succeed
    if(!create_point_evaluation(left, left_eval, contacts_F, zero_terms_T))
    {
        return false;
    }

    do
    {
        variables_evaluations_block right_eval(variables_mask_t(0));
        if(!create_point_evaluation(right, right_eval, contacts_F, zero_terms_T))
        {
            return false;
        }

        do
        {
            if(is_contacts_F_connectivity_rule_satisfied(contacts_F, left_eval, right_eval))
            {
                points_.push_back(std::move(left_eval));
                points_.push_back(std::move(right_eval));
                return true;
            }
        } while(generate_next_point_evaluation(right, right_eval, contacts_F, zero_terms_T));
        TERMINATE_IF_NEEDED();
    } while(generate_next_point_evaluation(left, left_eval, contacts_F, zero_terms_T));

    return false;
}

auto model::does_point_evaluation_satisfies_basic_rules(const term* t,
                                                        const variables_evaluations_block& evaluation,
                                                        const formulas_t& contacts_F,
                                                        const terms_t& zero_terms_T) const -> bool
{
    return t->evaluate(evaluation).is_constant_true() &&
           are_zero_terms_T_satisfied(zero_terms_T, evaluation) &&
           is_contacts_F_reflexive_rule_satisfied(contacts_F, evaluation);
}

auto model::is_contacts_F_connectivity_rule_satisfied(const formulas_t& contacts_F,
                                                      const variables_evaluations_block& eval_a,
                                                      const variables_evaluations_block& eval_b) const -> bool
{
    for(const auto& c : contacts_F)
    {
        const auto l = c->get_left_child_term();
        const auto r = c->get_right_child_term();
        if((l->evaluate(eval_a).is_constant_true() &&
            r->evaluate(eval_b).is_constant_true()) ||
           (l->evaluate(eval_b).is_constant_true() &&
            r->evaluate(eval_a).is_constant_true()))
        {
            return false;
        }
    }
    return true;
}

auto model::are_zero_terms_T_satisfied(const terms_t& zero_terms_T,
                                       const variables_evaluations_block& evaluation) const -> bool
{
    for(const auto& z : zero_terms_T)
    {
        if(!z->evaluate(evaluation).is_constant_false())
        {
            return false;
        }
    }
    return true;
}

auto model::is_contacts_F_reflexive_rule_satisfied(const formulas_t& contacts_F,
                                                   const variables_evaluations_block& evaluation) const
    -> bool
{
    for(const auto& c : contacts_F)
    {
        if(c->get_left_child_term()->evaluate(evaluation).is_constant_true() &&
           c->get_right_child_term()->evaluate(evaluation).is_constant_true())
        {
            return false;
        }
    }
    return true;
}

auto model::construct_point(const formulas_t& contacts_F, const terms_t& zero_terms_T) -> bool
{
    term t(mgr_);
    t.construct_constant(true);

    variables_evaluations_block eval(variables_mask_t(0)); // it will be overridden if succeed
    if(create_point_evaluation(&t, eval, contacts_F, zero_terms_T))
    {
        points_.push_back(std::move(eval));
        return true;
    }

    return false;
}

void model::calculate_the_model_evaluation_of_each_variable()
{
    const auto points_size = points_.size();
    variable_evaluations_.clear();
    variable_evaluations_.resize(
        mgr_->get_variables().size(),
        model_points_set_t(points_size)); // initialize each variable evaluation as the empty set

    // Calculate the MODEL evaluation of each variable, i.e. each variable_id
    // v(Pi) = { point | point_evaluation[Pi] == 1 } , i.e. the evaluation of variable with id Pi is 1, i.e.
    // the bit at position Pi is 1
    for(size_t point = 0; point < points_size; ++point)
    {
        const auto& point_evaluation = points_[point].get_evaluations();

        // iterate only set bits(1s)
        auto Pi = point_evaluation.find_first();
        while(Pi != variables_evaluations_t::npos)
        {
            variable_evaluations_[Pi].set(point); // adds the point to the v(Pi) set
            Pi = point_evaluation.find_next(Pi);
        }
    }
}

auto model::print(std::ostream& out) const -> std::ostream&
{
    out << "Model points: \n";
    for(size_t i = 0; i < points_.size(); ++i)
    {
        out << std::to_string(i) << " : ";
        mgr_->print(out, points_[i]);
        out << "\n";
    }

    return out;
}
