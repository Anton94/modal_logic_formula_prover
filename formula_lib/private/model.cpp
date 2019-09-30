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

    initialize_contact_relations_from_contacts();
    calculate_the_model_evaluation_of_each_variable();
    calculate_contact_relations();

    return true;
}

auto model::generate_next() -> bool
{
    // TODO: maybe cache the valid evaluations per term
    // simulation of +1 operation from left to right but not just 0/1 bits but all combinations per element
    for (auto& point : points_)
    {
        if (generate_next_positive_evaluation(point.t, point.evaluation))
        {
            // TODO: this maybe can be done smarter and not just whole new calculation
            calculate_the_model_evaluation_of_each_variable();
            calculate_contact_relations();
            return true;
        }

        // resetting the evaluation for that point, by constructing it again
        const auto is_reset = construct_non_zero_term_evaluation(point.t, point.evaluation);
        assert(is_reset);
        (void)is_reset;
    }
    return false;
}

auto model::is_in_contact(const term* a, const term* b) const -> bool
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

    for (size_t i = 0; i < number_of_contacts_; ++i)
    {
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
    }
    return false;
}

auto model::is_not_in_contact(const term* a, const term* b) const -> bool
{
    return !is_in_contact(a, b);
}

auto model::is_empty_set(const term* a) const -> bool
{
    return a->evaluate(variable_evaluations_, points_.size()).none();
}

auto model::is_not_empty_set(const term* a) const -> bool
{
    return !is_empty_set(a);
}

auto model::get_model_points() const -> const points_t&
{
    return points_;
}

auto model::get_variables_evaluations() const -> const variable_id_to_points_t&
{
    return variable_evaluations_;
}

auto model::get_number_of_contacts() const -> size_t
{
    return number_of_contacts_;
}

auto model::get_number_of_non_zeros() const -> size_t
{
    return points_.size() - get_number_of_contact_points();
}

auto model::get_number_of_contact_points() const -> size_t
{
    return get_number_of_contacts() * 2;
}

auto model::get_number_of_non_zero_points() const -> size_t
{
    return get_number_of_non_zeros();
}

auto model::get_contacts() const -> const contacts_t&
{
    return contact_relations_;
}

void model::clear()
{
    mgr_ = nullptr;
    used_variables_.clear();
    number_of_contacts_ = 0;
    points_.clear();
    contact_relations_.clear();
    variable_evaluations_.clear();
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
        points_.push_back({ left, std::move(left_evaluation) });
        points_.push_back({ right, std::move(right_evaluation) });
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
        points_.push_back({ z, std::move(eval) });
    }

    return true;
}

void model::initialize_contact_relations_from_contacts()
{
    const auto n = points_.size();
    contact_relations_from_contacts_.resize(n, model_points_set_t(n, false)); // construct a square zero matrix of size @n

    // We have constructed the contact model points in a sequentional order,
    // so point at position 2k is connected with point at position 2k+1 (each point at an even position is connected with the odd point right next to it)
    for (size_t i = 0; i < number_of_contacts_; ++i)
    {
        const auto l_contact_point = i * 2;
        const auto r_contact_point = i * 2 + 1;
        contact_relations_from_contacts_[l_contact_point].set(r_contact_point);
        contact_relations_from_contacts_[r_contact_point].set(l_contact_point);
    }
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
        if (!evaluation.generate_next_evaluation()) // TODO the generation can be done smarter, e.g. when the evaluation is false, generate new variable evaluations just for the varaibles in @t.
        {
            return false;
        }
    } while (!t->evaluate(evaluation).is_constant_true());
    return true;
}

void model::calculate_the_model_evaluation_of_each_variable()
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

void model::calculate_contact_relations()
{
    contact_relations_ = contact_relations_from_contacts_;

    // TODO: connectivity axiome can add some more contacts
}

std::ostream& operator<<(std::ostream& out, const model& m)
{
    out << "Model points: \n";
    for (size_t i = 0; i < m.points_.size(); ++i)
    {
        out << std::to_string(i) << " : ";
        m.mgr_->print(out, m.points_[i].evaluation);
    }

    // TODO: contact connections print
    out << "First " << m.number_of_contacts_ * 2 << " points are connected, i.e. 2k and 2k+1";

    out << "Model evaluation of the variables";
    for (size_t i = 0; i < m.variable_evaluations_.size(); ++i)
    {
        const auto& variable_evaluation_bitset = m.variable_evaluations_[i];

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