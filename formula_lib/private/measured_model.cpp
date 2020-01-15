#include "measured_model.h"
#include "formula.h"
#include "formula_mgr.h"
#include "term.h"
#include "thread_termiator.h"

#include <algorithm>
#include <cassert>

measured_model::measured_model()
    : system_(0)
{
}

measured_model::~measured_model() = default;

auto measured_model::create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
            const terms_t& zero_terms_F, const formulas_t& measured_less_eq_T, const formulas_t& measured_less_eq_F, const variables_mask_t& used_variables, const formula_mgr* mgr)
    -> bool
{
    clear();
    mgr_ = mgr;
    used_variables_ = used_variables;
    number_of_contacts_ = contacts_T.size();

    measured_less_eq_T_ = measured_less_eq_T;
    measured_less_eq_F_ = measured_less_eq_F;

    if (!construct_contact_model_points(contacts_T) || !construct_non_zero_model_points(zero_terms_F))
    {
        trace() << "Unable to construct the model points with binary var. evaluations which evaluates the contact & non-zero terms to 1";
        return false;
    }

    // We need at least one modal point in the model
    // and additional one point per atomic measured formula.
    const auto additional_points_for_measured_atomics = measured_less_eq_T.size() + measured_less_eq_F.size();
    const auto total_points_count = std::max(1ul, static_cast<unsigned long>(points_.size() + additional_points_for_measured_atomics));
    if(points_.size() < total_points_count)
    {
        const auto number_of_points_to_add = total_points_count - points_.size();
        info() << "Adding additional " << number_of_points_to_add << " points because of the <=m/~<=m atomics. "
                  "We need a potential one model point for each side of each inequality in the system.";
        constant_true_ = std::make_unique<term>(mgr_);
        constant_true_->construct_constant(true);
        points_.insert(points_.end(), number_of_points_to_add, {constant_true_.get(), variables_evaluations_block_for_positive_term(*constant_true_, used_variables_)});
    }

    create_contact_relations_first_2k_in_contact(points_.size(), contacts_T.size());
    calculate_the_model_evaluation_of_each_variable();

    system_ = system_of_inequalities(points_.size());
    while (!is_zero_term_rule_satisfied(zero_terms_T) || !is_contact_F_rule_satisfied(contacts_F) ||
           !has_solvable_system_of_inequalities())
    {
        TERMINATE_IF_NEEDED();
        if (!generate_next())
        {
            trace() << "Unable to generate a new combination of binary var. evaluations for the model points.";
            return false;
        }
    }

    return true;
}

auto measured_model::generate_next() -> bool
{
    // TODO: maybe cache the valid evaluations per term
    // simulation of +1 operation from left to right but not just 0/1 bits but all combinations per element
    for (auto& point : points_)
    {
        if (point.evaluation.generate_next_evaluation())
        {
            // TODO: this maybe can be done smarter and not just whole new calculation
            calculate_the_model_evaluation_of_each_variable();
            return true;
        }

        const auto is_reset = point.evaluation.reset();
        assert(is_reset);
        (void)is_reset;
    }
    return false;
}

auto measured_model::is_contact_F_rule_satisfied(const formulas_t& contacts_F) const -> bool
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

auto measured_model::is_zero_term_rule_satisfied(const terms_t& zero_terms_T) const -> bool
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

auto measured_model::has_solvable_system_of_inequalities() -> bool
{
    // For each <=m(a,b) calculate v(a) and v(b), then we will create
    // an inequality of the following type: SUM_I Xi <= SUM_J Xj, where I is v(a) and J is v(b).
    // For each ~<=m(a,b) calculate v(a) and v(b), then we will create
    // an inequality of the following type: SUM_I Xi > SUM_J Xj, where I is v(a) and J is v(b).
    // Each inequality is a row in the system of inequalities. If this system has a solution, then we are good.

    const auto points_size = points_.size();
    system_.clear();

    for(const auto& m : measured_less_eq_T_)
    {
        const auto v_a = m->get_left_child_term()->evaluate(variable_evaluations_, points_size);
        const auto v_b = m->get_right_child_term()->evaluate(variable_evaluations_, points_size);
        if (!system_.add_constraint(v_a, v_b, system_of_inequalities::inequality_operation::LE))
        {
            verbose() << "Unable to find a solution for the system when adding the constraint for " << *m << "\n" << *static_cast<imodel* const>(this); // TODO: why not able to call directly *this??? why unresolved ???
            return false;
        }
    }

    for(const auto& m : measured_less_eq_F_)
    {
        const auto v_a = m->get_left_child_term()->evaluate(variable_evaluations_, points_size);
        const auto v_b = m->get_right_child_term()->evaluate(variable_evaluations_, points_size);
        if (!system_.add_constraint(v_a, v_b, system_of_inequalities::inequality_operation::G))
        {
            std::stringstream out;
            print_system(out);
            verbose() << "Unable to find a solution for the system when adding the constraint for ~" << *m << "\n" << *static_cast<imodel* const>(this);
            return false;
        }
    }

    info() << "Found a solution for the system:\n" << *static_cast<imodel* const>(this);
    return true;
}

auto measured_model::is_in_contact(const term* a, const term* b) const -> bool
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

auto measured_model::is_not_in_contact(const term* a, const term* b) const -> bool
{
    return !is_in_contact(a, b);
}

auto measured_model::is_empty_set(const term* a) const -> bool
{
    return a->evaluate(variable_evaluations_, points_.size()).none();
}

auto measured_model::is_not_empty_set(const term* a) const -> bool
{
    return !is_empty_set(a);
}

auto measured_model::get_model_points() const -> const points_t&
{
    return points_;
}

void measured_model::clear()
{
    used_variables_.clear();
    number_of_contacts_ = 0;
    constant_true_.reset();
    points_.clear();
    measured_less_eq_T_.clear();
    measured_less_eq_F_.clear();
    system_.clear();

    imodel::clear();
}

auto measured_model::construct_contact_model_points(const formulas_t& contacts) -> bool
{
    // For each contact term it will generate an evaluation of all used variables which evaluates it to the constant_true
    for (const auto& c : contacts)
    {
        const term* left = c->get_left_child_term();
        const term* right = c->get_right_child_term();
        variables_evaluations_block_for_positive_term left_evaluation(*left, used_variables_); // it will be overriten if succeed
        variables_evaluations_block_for_positive_term right_evaluation(*right, used_variables_);
        if (!left_evaluation.reset() || !right_evaluation.reset())
        {
            return false;
        }
        points_.push_back({ left, std::move(left_evaluation) });
        points_.push_back({ right, std::move(right_evaluation) });
    }

    return true;
}

auto measured_model::construct_non_zero_model_points(const terms_t& non_zero_terms) -> bool
{
    // For each non-zero term it will generate an evaluation of all used variables which evaluates it to the constant_true
    for (const auto& z : non_zero_terms)
    {
        variables_evaluations_block_for_positive_term eval(*z, used_variables_); // it will be overriten if succeed
        if (!eval.reset())
        {
            return false;
        }
        points_.push_back({ z, std::move(eval) });
    }

    return true;
}

void measured_model::calculate_the_model_evaluation_of_each_variable()
{
    const auto points_size = points_.size();
    variable_evaluations_.clear();
    variable_evaluations_.resize(mgr_->get_variables().size(), model_points_set_t(points_size)); // initialize each variable evaluation as the empty set

    // Calculate the MODEL evaluation of each variable, i.e. each variable_id
    // v(Pi) = { point | point_evaluation[Pi] == 1 } , i.e. the evaluation of variable with id Pi is 1, i.e. the bit at position Pi is 1
    for (size_t point = 0; point < points_size; ++point)
    {
        const auto& point_evaluation = points_[point].evaluation.get_evaluations_block().get_evaluations();

        // iterate only set bits(1s)
        auto Pi = point_evaluation.find_first();
        while (Pi != variables_evaluations_t::npos)
        {
            variable_evaluations_[Pi].set(point); // adds the point to the v(Pi) set
            Pi = point_evaluation.find_next(Pi);
        }
    }
}

auto measured_model::print_system_sum_variables(std::ostream& out, const model_points_set_t& variables) const -> std::ostream&
{
    // iterate only set bits(1s)
    auto var_id = variables.find_first();
    bool has_printed_first = false;
    if(var_id == variables_evaluations_t::npos)
    {
        out << "0";
        return out;
    }

    while (var_id != variables_evaluations_t::npos)
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

auto measured_model::print_system(std::ostream& out) const -> std::ostream&
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

auto measured_model::print(std::ostream& out) const -> std::ostream&
{
    out << "Model points: \n";
    const auto points_size = points_.size();
    for(size_t i = 0; i < points_size; ++i)
    {
        out << std::to_string(i) << " : ";
        mgr_->print(out, points_[i].evaluation.get_evaluations_block());
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
