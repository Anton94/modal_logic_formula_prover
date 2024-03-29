#include "model.h"
#include "formula.h"
#include "term.h"
#include "logger.h"
#include "utils.h"
#include "../include/thread_termiator.h"

#include <cassert>

auto model::create(const formulas_t& contacts_T, const formulas_t& contacts_F,
                   const terms_t& zero_terms_T,  const terms_t& zero_terms_F,
                   const formulas_t& measured_less_eq_T, const formulas_t& measured_less_eq_F,
                   const variables_mask_t& used_variables, const variables_t& variable_names) -> bool
{
    trace() << "Start creating a model.";

    clear();

    used_variables_ = used_variables;
    variable_names_ = variable_names;

    trace() << "Used variables are " << used_variables_.count() << ". Total variables in the formula are " << used_variables_.size() << ".";

    const auto expected_maximal_number_of_modal_points = std::max(1ul, contacts_T.size() * 2 + zero_terms_F.size());
    construct_contact_matrix(expected_maximal_number_of_modal_points);

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

    // Reduce the contact matrix to the used modal points count
    reduce_contact_matrix(points_.size());

    trace() << "Found a model:\n" << *this;

    return true;
}

void model::clear()
{
    used_variables_.clear();

    imodel::clear();
}

void model::construct_contact_matrix(size_t modal_points_count)
{
    contact_relations_.clear();
    contact_relations_.resize(modal_points_count, model_points_set_t(modal_points_count)); // Fill NxN matrix with 0s

    // Add the reflexivity of each modal point.
    for(size_t i = 0; i < modal_points_count; ++i)
    {
        contact_relations_[i].set(i);
    }
}

void model::add_contact(const point_t& a, const point_t& b)
{
    const auto index_a = get_point_index(a);
    const auto index_b = get_point_index(b);
    contact_relations_[index_a].set(index_b);
    contact_relations_[index_b].set(index_a);
}

void model::reduce_contact_matrix(size_t modal_points_count)
{
    contact_relations_.resize(modal_points_count);
    for(auto& contct_relation : contact_relations_)
    {
        contct_relation.resize(modal_points_count);
    }
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
        add_point(eval);
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
        // variable evaluations just for the varaibles in @t. Maybe the variables_evaluations_block_for_positive_term?
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
                add_point(left_eval);
                add_point(right_eval);
                add_contact(left_eval, right_eval);

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
    if(t && !t->evaluate(evaluation).is_constant_true())
    {
        return false;
    }

    return are_zero_terms_T_satisfied(zero_terms_T, evaluation) &&
           is_contacts_F_reflexive_rule_satisfied(contacts_F, evaluation);
}

auto model::construct_point(const formulas_t& contacts_F, const terms_t& zero_terms_T) -> bool
{
    variables_evaluations_block eval(variables_mask_t(0)); // it will be overridden if succeed
    if(create_point_evaluation(nullptr, eval, contacts_F, zero_terms_T))
    {
        add_point(eval);
        return true;
    }

    return false;
}

auto model::add_point(const point_t& point) -> bool
{
    const auto it = std::find(points_.begin(), points_.end(), point);
    if(it != points_.end())
    {
        return false;
    }

    points_.push_back(point);
    return true;
}

auto model::get_point_index(const point_t& point) -> size_t
{
    const auto it = std::find(points_.begin(), points_.end(), point);
    assert(it != points_.end());
    return size_t(std::distance(points_.begin(), it));
}

void model::calculate_the_model_evaluation_of_each_variable()
{
    const auto points_size = points_.size();
    variable_evaluations_.clear();
    variable_evaluations_.resize(
        used_variables_.size(),
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
