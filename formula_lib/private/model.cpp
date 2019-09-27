#include "model.h"
#include "formula.h"
#include "formula_mgr.h"
#include "term.h"

#include <cassert>

auto model::construct_model_points(const formulas_t& contacts, const terms_t& non_zero_terms, const variables_mask_t& used_variables, const formula_mgr* mgr) -> bool
{
    mgr_ = mgr;
    used_variables_ = used_variables;
    number_of_contacts_ = contacts.size();

    if (!construct_contact_model_points(contacts) || !construct_non_zero_model_points(non_zero_terms))
    {
        return false;
    }
    calculate_the_model_evaluation_of_each_variable();
    return true;
}

auto model::generate_next_model_points_evaluation_combination() -> bool
{
    // TODO: explain more
    // +1 operation from left to right but not just 0/1 bits but all combinations per element
    for (auto& model_point : model_points_) // TODO: keep track of last and continue from it instead of always from the beginning
    {
        if (generate_next_positive_evaluation(model_point.t, model_point.evaluation))
        {
            // TODO: this maybe can be done smarter and not just whole new calculation
            calculate_the_model_evaluation_of_each_variable();
            return true;
        }

        // resetting the evaluation for that point, by constructing it again
        const auto is_reset = construct_non_zero_term_evaluation(model_point.t, model_point.evaluation);
        assert(is_reset);
        (void)is_reset;
    }
    return false;
}

auto model::is_in_contact(const term* a, const term* b) const -> bool
{
    // Note the following:
    // We have reflexive and symetric relation. The Contact implies to create two points, x and y, which are connected.
    // We have constructed the contact model points in a sequentional order, so point at position 2k is connected with point at position 2k+1 (each point at an even position is connected with the odd point right next to it)
    // i.e. [0, 1, 2, 3, 4, ...] model point 0 and 1 are connected, 2 and 3 are connected and so on.
    const auto eval_a = a->evaluate(variable_ids_model_evaluations_, model_points_.size());
    const auto eval_b = b->evaluate(variable_ids_model_evaluations_, model_points_.size());
    if (eval_a.none() || eval_b.none())
    {
        return false; // there is no way to be in contact if at least on of the evaluations is the empty set
    }
    if ((eval_a & eval_b).any())
    {
        return true; // left eval set and right eval set has a common point, but each point is reflexive so there is a contact between the two sets
    }
    // Now we know that there are no common model points in the two evaluations
    // Let's imagine that we have 6 contact points and 3 non-zero points, then the evaluations might be something like:
    // [0,1,0,1,0,1,  0,0,1] left_eval
    // [0,0,0,0,1,0,  1,1,0] right_eval
    // We have to check that there are no contacts between the two set's.
    // Note that each contact generated two points which are at position 2k and 2k+1
    // So, we have to check if one of the set's contain the point 2k(i.e. the bit at position 2k is set(1)) and the other set contain the point 2k+1
    // Will iterate them in the following order:
    // [{0,1},0,1,0,1,  0,0,1] [0,1,{0,1},0,1,  0,0,1] [0,1,0,1,{0,1},  0,0,1]
    // [{0,0},0,0,1,0,  1,1,0] [0,0,{0,0},1,0,  1,1,0] [0,0,0,0,{1,0},  1,1,0]
    // In the above example there is a contact betwen them in last pair
    // TODO: this maybe can be done smarter!
    for (size_t i = 0; i < number_of_contacts_; ++i)
    {
        const auto l_contact_point = i * 2;
        const auto r_contact_point = i * 2 + 1;
        if ((eval_a.test(l_contact_point) && eval_b.test(r_contact_point)) ||
            (eval_a.test(r_contact_point) && eval_b.test(l_contact_point)))
        {
            return true;
        }
    }
    return false;
}

auto model::is_not_in_contact(const term* a, const term* b) const -> bool
{
    return !is_in_contact(a, b);
}

auto model::is_empty_set(const term* a) const -> bool
{
    return a->evaluate(variable_ids_model_evaluations_, model_points_.size()).none();
}

auto model::is_not_empty_set(const term* a) const -> bool
{
    return !is_empty_set(a);
}

auto model::get_model_points() const -> const model_points_t&
{
    return model_points_;
}

auto model::get_variables_evaluations() const -> const variable_id_to_model_points_t&
{
    return variable_ids_model_evaluations_;
}

auto model::get_number_of_contacts() const -> size_t
{
    return number_of_contacts_;
}

void model::clear()
{
    used_variables_.clear();
    model_points_.clear();
    variable_ids_model_evaluations_.clear();
}

auto model::construct_contact_model_points(const formulas_t& contacts) -> bool
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
        model_points_.push_back({ left, std::move(left_evaluation) });
        model_points_.push_back({ right, std::move(right_evaluation) });
    }

    return true;
}

auto model::construct_non_zero_model_points(const terms_t& non_zero_terms) -> bool
{
    // For each non-zero term it will generate an evaluation of all used variables which evaluates it to the constant_true
    for (const auto& z : non_zero_terms)
    {
        variables_evaluations_block eval(variables_mask_t(0)); // it will be overriten if succeed
        if (!construct_non_zero_term_evaluation(z, eval))
        {
            return false;
        }
        model_points_.push_back({ z, std::move(eval) });
    }

    return true;
}

auto model::construct_non_zero_term_evaluation(const term* t, variables_evaluations_block& out_evaluation) const -> bool
{
    out_evaluation = variables_evaluations_block(used_variables_);

    // the evaluation of the term should be the constant true, i.e.
    // for t != 0 : the evaluation of 't' with the given @evaluation should be non-zero
    return t->evaluate(out_evaluation).is_constant_true() ||
        generate_next_positive_evaluation(t, out_evaluation);
}

auto model::generate_next_positive_evaluation(const term* t, variables_evaluations_block& evaluation) const -> bool
{
    do
    {
        if (!evaluation.generate_next_evaluation()) // TODO the generation can be done smarter, e.g. when the evaluatio is false, generate new variable evaluations just for the varaibles in @t.
        {
            return false;
        }
    } while (!t->evaluate(evaluation).is_constant_true());
    return true;
}

void model::calculate_the_model_evaluation_of_each_variable()
{
    const auto model_points_size = model_points_.size();
    variable_ids_model_evaluations_.clear();
    variable_ids_model_evaluations_.resize(mgr_->get_variables().size(), model_points_set_t(model_points_size)); // initialize each variable evaluation as the empty set

    // Calculate the MODEL evaluation of each variable, i.e. each variable_id
    // v(Pi) = { model_point | model_point_evaluation[Pi] == 1 } , i.e. the evaluation of variable with id Pi is 1, i.e. the bit at position Pi is 1
    for (size_t model_point_index = 0; model_point_index < model_points_size; ++model_point_index)
    {
        const auto& model_point_evaluation = model_points_[model_point_index].evaluation.get_evaluations();

        // iterate only set bits(1s)
        // variable_ids_model_evaluations_[Pi] is v(Pi)
        auto Pi = model_point_evaluation.find_first();
        while (Pi != variables_evaluations_t::npos)
        {
            variable_ids_model_evaluations_[Pi].set(model_point_index);
            Pi = model_point_evaluation.find_next(Pi);
        }
    }
}

std::ostream& operator<<(std::ostream& out, const model& m)
{
    out << "Model points: \n";
    for (size_t i = 0; i < m.model_points_.size(); ++i)
    {
        out << std::to_string(i) << " : ";
        m.mgr_->print(out, m.model_points_[i].evaluation);
    }

    // TODO: contact connections print
    out << "First " << m.number_of_contacts_ * 2 << " points are connected, i.e. 2k and 2k+1";

    out << "Model evaluation of the variables";
    for (size_t i = 0; i < m.variable_ids_model_evaluations_.size(); ++i)
    {
        const auto& variable_evaluation_bitset = m.variable_ids_model_evaluations_[i];

        out << "v(" << m.mgr_->get_variable(i) << ") = { ";

        auto model_point_id = variable_evaluation_bitset.find_first();
        while (model_point_id != variables_evaluations_t::npos)
        {
            out << std::to_string(model_point_id) << ", ";
            model_point_id = variable_evaluation_bitset.find_next(model_point_id);
        }
        out << "}";
    }

    return out;
}