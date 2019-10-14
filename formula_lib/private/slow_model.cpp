#include "slow_model.h"
#include "formula.h"
#include "formula_mgr.h"
#include "term.h"

#include <cassert>

auto slow_model::create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
            const terms_t& zero_terms_F, const variables_mask_t& used_variables, const formula_mgr* mgr)
    -> bool
{
    clear();
    mgr_ = mgr;
    used_variables_ = used_variables;
    number_of_contacts_ = contacts_T.size();

    if (!construct_contact_model_points(contacts_T) || !construct_non_zero_model_points(zero_terms_F))
    {
        trace() << "Unable to construct the model points with binary var. evaluations which evaluates the contact & non-zero terms to 1";
        return false;
    }

    create_contact_relations_first_2k_in_contact(points_.size(), contacts_T.size());
    calculate_the_model_evaluation_of_each_variable();

    while (!is_zero_term_rule_satisfied(zero_terms_T) || !is_contact_F_rule_satisfied(contacts_F))
    {
        if (!generate_next())
        {
            trace() << "Unable to generate a new combination of binary var. evaluations for the model points.";
            return false;
        }
    }

    return true;
}

auto slow_model::generate_next() -> bool
{
    // TODO: maybe cache the valid evaluations per term
    // simulation of +1 operation from left to right but not just 0/1 bits but all combinations per element
    for (auto& point : points_)
    {
        if (generate_next_positive_evaluation(point.t, point.evaluation))
        {
            // TODO: this maybe can be done smarter and not just whole new calculation
            calculate_the_model_evaluation_of_each_variable();
            return true;
        }

        // resetting the evaluation for that point, by constructing it again
        const auto is_reset = construct_non_zero_term_evaluation(point.t, point.evaluation);
        assert(is_reset);
        (void)is_reset;
    }
    return false;
}

auto slow_model::is_contact_F_rule_satisfied(const formulas_t& contacts_F) const -> bool
{
    for (const auto& c : contacts_F)
    {
        if (is_in_contact(c->get_left_child_term(), c->get_right_child_term()))
        {
            return false;
        }
    }
    return true;
}

auto slow_model::is_zero_term_rule_satisfied(const terms_t& zero_terms_T) const -> bool
{
    // The zero term, i.e. the <=(a,b) operation which is translated to (a * -b = 0),
    // has the following semantic: <=(a,b) is satisfied iif v(a * -b) = empty_set which is a bitset of only zeros
    // v(a * -b) = v(a) & v(-b) = v(a) & v(W\v(b)) = v(a) & ~v(b)
    for (const auto& z : zero_terms_T)
    {
        if (is_not_empty_set(z))
        {
            return false;
        }
    }
    return true;
}

auto slow_model::is_in_contact(const term* a, const term* b) const -> bool
{
    const auto eval_a = a->evaluate(variable_evaluations_, points_.size());
    const auto eval_b = b->evaluate(variable_evaluations_, points_.size());
    if (eval_a.none() || eval_b.none())
    {
        return false; // there is no way to be in contact if at least on of the evaluations is the empty set
    }
    if ((eval_a & eval_b).any())
    {
        return true; // left eval set and right eval set has a common point, but each point is reflexive so there is a contact between the two sets
    }

    auto point_in_eval_a = eval_a.find_first(); // TODO: iterate over the bitset with less set bits
    while (point_in_eval_a != model_points_set_t::npos)
    {
        const auto& points_in_contact_with_point_in_eval_a = contact_relations_[point_in_eval_a];
        if ((points_in_contact_with_point_in_eval_a & eval_b).any())
        {
            return true;
        }
        point_in_eval_a = eval_a.find_next(point_in_eval_a);
    }
    return false;
}

auto slow_model::is_not_in_contact(const term* a, const term* b) const -> bool
{
    return !is_in_contact(a, b);
}

auto slow_model::is_empty_set(const term* a) const -> bool
{
    return a->evaluate(variable_evaluations_, points_.size()).none();
}

auto slow_model::is_not_empty_set(const term* a) const -> bool
{
    return !is_empty_set(a);
}

auto slow_model::get_model_points() const -> const points_t&
{
    return points_;
}

void slow_model::clear()
{
    used_variables_.clear();
    number_of_contacts_ = 0;
    points_.clear();

    imodel::clear();
}

auto slow_model::construct_contact_model_points(const formulas_t& contacts) -> bool
{
    // For each contact term it will generate an evaluation of all used variables which evaluates it to the constant_true
    for (const auto& c : contacts)
    {
        variables_evaluations_block left_evaluation(variables_mask_t(0)); // it will be overriten if succeed
        variables_evaluations_block right_evaluation(variables_mask_t(0));
        const term* left = c->get_left_child_term();
        const term* right = c->get_right_child_term();
        if (!construct_non_zero_term_evaluation(left, left_evaluation) ||
            !construct_non_zero_term_evaluation(right, right_evaluation))
        {
            return false;
        }
        points_.push_back({ left, std::move(left_evaluation) });
        points_.push_back({ right, std::move(right_evaluation) });
    }

    return true;
}

auto slow_model::construct_non_zero_model_points(const terms_t& non_zero_terms) -> bool
{
    // For each non-zero term it will generate an evaluation of all used variables which evaluates it to the constant_true
    for (const auto& z : non_zero_terms)
    {
        variables_evaluations_block eval(variables_mask_t(0)); // it will be overriten if succeed
        if (!construct_non_zero_term_evaluation(z, eval))
        {
            return false;
        }
        points_.push_back({ z, std::move(eval) });
    }

    return true;
}

auto slow_model::construct_non_zero_term_evaluation(const term* t, variables_evaluations_block& out_evaluation) const -> bool
{
    out_evaluation = variables_evaluations_block(used_variables_);

    // the evaluation of the term should be the constant true, i.e.
    // for t != 0 : the evaluation of 't' with the given @evaluation should be non-zero
    return t->evaluate(out_evaluation).is_constant_true() ||
        generate_next_positive_evaluation(t, out_evaluation);
}

auto slow_model::generate_next_positive_evaluation(const term* t, variables_evaluations_block& evaluation) const -> bool
{
    do
    {
        if (!evaluation.generate_next_evaluation()) // TODO the generation can be done smarter, e.g. when the evaluation is false, generate new variable evaluations just for the varaibles in @t.
        {
            return false;
        }
    } while (!t->evaluate(evaluation).is_constant_true());
    return true;
}

void slow_model::calculate_the_model_evaluation_of_each_variable()
{
    const auto points_size = points_.size();
    variable_evaluations_.clear();
    variable_evaluations_.resize(mgr_->get_variables().size(), model_points_set_t(points_size)); // initialize each variable evaluation as the empty set

    // Calculate the MODEL evaluation of each variable, i.e. each variable_id
    // v(Pi) = { point | point_evaluation[Pi] == 1 } , i.e. the evaluation of variable with id Pi is 1, i.e. the bit at position Pi is 1
    for (size_t point = 0; point < points_size; ++point)
    {
        const auto& point_evaluation = points_[point].evaluation.get_evaluations();

        // iterate only set bits(1s)
        auto Pi = point_evaluation.find_first();
        while (Pi != variables_evaluations_t::npos)
        {
            variable_evaluations_[Pi].set(point); // adds the point to the v(Pi) set
            Pi = point_evaluation.find_next(Pi);
        }
    }
}

auto slow_model::print(std::ostream& out) const -> std::ostream&
{
    out << "Model points: \n";
    for(size_t i = 0; i < points_.size(); ++i)
    {
        out << std::to_string(i) << " : ";
        mgr_->print(out, points_[i].evaluation);
        out << "\n";
    }

    return out;
}
