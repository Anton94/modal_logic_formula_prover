#include "measured_model.h"
#include "formula.h"
#include "term.h"
#include "logger.h"
#include "thread_termiator.h"
#include "utils.h"

#include <algorithm>
#include <cassert>
#include <iomanip>

namespace
{

auto is_contact_satisfied(const formula* contact, const model_points_set_t& points, const term_to_modal_points_t& term_evaluations, const contacts_t& contact_relations) -> bool
{
    const auto a = contact->get_left_child_term();
    const auto b = contact->get_right_child_term();
    const auto eval_it_a = term_evaluations.find(a);
    const auto eval_it_b = term_evaluations.find(b);
    assert(eval_it_a != term_evaluations.end());
    assert(eval_it_b != term_evaluations.end());

    // The evaluations contains all valid modal points, so restrict them to the provided subset @points.
    const auto eval_a = eval_it_a->second & points;
    const auto eval_b = eval_it_b->second & points;
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
        const auto& points_in_contact_with_point_in_eval_a = contact_relations[point_in_eval_a];
        if ((points_in_contact_with_point_in_eval_a & eval_b).any())
        {
            return true;
        }
        point_in_eval_a = eval_a.find_next(point_in_eval_a);
    }
    return false;
}

auto are_contacts_T_satisfied(const formulas_t& contacts_T,
                              const model_points_set_t& points,
                              const term_to_modal_points_t& term_evaluations,
                              const contacts_t& contact_relations) -> bool
{
    for(const auto& c : contacts_T)
    {
        if(!is_contact_satisfied(c, points, term_evaluations, contact_relations))
        {
            return false;
        }
    }

    return true;
}

auto are_zero_terms_F_satisfied(const terms_t& zero_terms_F,
                                const model_points_set_t& points,
                                const term_to_modal_points_t& term_evaluations) -> bool
{
    for(const auto& t : zero_terms_F)
    {
        const auto eval_it = term_evaluations.find(t);
        assert(eval_it != term_evaluations.end());

        const auto eval = eval_it->second & points;
        if(!eval.any())
        {
            return false;
        }
    }
    return true;
}

}

measured_model::measured_model(size_t max_valid_modal_points_count) :
    max_valid_modal_points_count_(max_valid_modal_points_count),
    system_(0)
{
}

measured_model::~measured_model() = default;

auto measured_model::create(const formulas_t& contacts_T, const formulas_t& contacts_F,
                            const terms_t& zero_terms_T,  const terms_t& zero_terms_F,
                            const formulas_t& measured_less_eq_T, const formulas_t& measured_less_eq_F,
                            const variables_mask_t& used_variables, const variables_t& variable_names) -> bool
{
    trace() << "Start creating a measured model.";

    clear();

    variable_names_ = variable_names;
    used_variables_ = used_variables;

    measured_less_eq_T_ = measured_less_eq_T;
    measured_less_eq_F_ = measured_less_eq_F;

    constexpr const auto max_used_variables_support = 25;
    if(used_variables_.count() >= max_used_variables_support)
    {
        trace() << "Used variables are more than " << max_used_variables_support << ". Unable to calculate for so many variables.";
        return false;
    }

    trace() << "Used variables are " << used_variables_.count() << ". Total variables in the formula are " << used_variables_.size() << ".";

    const auto all_valid_points = construct_all_valid_points(used_variables_, contacts_F, zero_terms_T);
    if(all_valid_points.empty())
    {
        trace() << "Unable to create even one model point which does not break the =0 or the reflexivity of ~C atomics.";
        return false;
    }

    trace() << "Constructed " << all_valid_points.size() << " valid modal points. "
            << "This points are such that the zero terms and the reflexivity of non-contacts are satisfied.";

    if(all_valid_points.size() > max_valid_modal_points_count_)
    {
        trace() << "Unable to create the model because the valid modal points are " << all_valid_points.size() << ".\n"
                << "They are more than the preset maximal of " << max_valid_modal_points_count_ << ".\n"
                << "Add more zero terms/non-contacts to reduce the number of valid modal points.";
        return false;
    }

    const auto variable_evaluations_over_all_valid_points = generate_variable_evaluations(all_valid_points, used_variables_.size());

    const auto all_valid_contact_relations = build_contact_relations_matrix(contacts_F, variable_evaluations_over_all_valid_points);

    trace() << "Constructed all valid connections betweeen the valid modal points and variable evaluations."
            << "This connections preserves the non-contacts satisfiability.";

    if(trace().is_enabled())
    {
        std::stringstream s;
        s << "All valid modal points:\n";
        imodel::print(s, all_valid_points);

        s << "All valid contact relations:\n";
        imodel::print_contacts(s, all_valid_contact_relations);

        s << "Variable evaluations:\n";
        imodel::print_evaluations(s, variable_evaluations_over_all_valid_points);

        trace() << s.str();
    }

    // So far, the zero terms and non-contacts are satisfied.
    // From now on, no new relations, neigther points will be added, therefore the zero terms and non-contacts satistfactionwill be preserved.

    // Cache all term evaluations which will be requested later on.
    term_to_modal_points_t term_evaluations;
    add_term_evaluations(term_evaluations, zero_terms_F, variable_evaluations_over_all_valid_points);
    add_term_evaluations(term_evaluations, contacts_T, variable_evaluations_over_all_valid_points);
    add_term_evaluations(term_evaluations, measured_less_eq_F, variable_evaluations_over_all_valid_points);
    add_term_evaluations(term_evaluations, measured_less_eq_T, variable_evaluations_over_all_valid_points);

    system_ = system_of_inequalities(all_valid_points.size());

    const auto total_combinations = size_t(std::pow(2, all_valid_points.size()));
    trace() << "Total combinations of subset of model points are: " << total_combinations;

    const auto all_points = ~model_points_set_t(all_valid_points.size());
    return generate_model(all_points, contacts_T, contacts_F, zero_terms_F, all_valid_points, all_valid_contact_relations, term_evaluations);
}

auto measured_model::generate_model(const model_points_set_t& points,
                                    const formulas_t& contacts_T,
                                    const formulas_t& contacts_F,
                                    const terms_t& zero_terms_F,
                                    const points_t& all_valid_points,
                                    const contacts_t& all_valid_contact_relations,
                                    const term_to_modal_points_t& term_evaluations) -> bool
{
    TERMINATE_IF_NEEDED();

    // Already processed.
    // TODO make a better algorithm which does not need to keep track of already generated subsets in order to generate subsets with decreasing size.
    if(processed_combinations_.find(points) != processed_combinations_.end())
    {
        return false;
    }

    processed_combinations_.insert(points);

    if(processed_combinations_.size() % 1000 == 0)
    {
        trace() << "Checked " << processed_combinations_.size() << " combinations of subsets of modal points.";
    }

    // If non-zero terms or contacts are not satisfied then this points does not produce a valid model.
    // Therefore, they could not produce a measured model neighter.
    // No subset of them could satisfy the contacts/zero terms because they require existnece of more points/relations but the set of points/relations is even reduced.
    if(!are_zero_terms_F_satisfied(zero_terms_F, points, term_evaluations) ||
       !are_contacts_T_satisfied(contacts_T, points, term_evaluations, all_valid_contact_relations))
    {
        return false;
    }

    // So far the subset @points of valid model points produces a valid model.
    // Check if satisfies the system.
    if(create_system_of_inequalities(points, term_evaluations, system_))
    {
        // Good. Found a measured model.
        save_as_model(points, contacts_F, all_valid_points);
        trace() << "Found a measured model:\n" << *this;
        return true;
    }

    if(points.count() <= 1) // The model should have at least one modal point
    {
        return false;
    }

    // Go through every subset with points_count - 1 elements and recursively check them.

    auto point_pos = points.find_first();
    while (point_pos != model_points_set_t::npos)
    {
        auto subset_points = points;
        subset_points.set(point_pos, false);

        if(generate_model(subset_points, contacts_T, contacts_F, zero_terms_F, all_valid_points, all_valid_contact_relations, term_evaluations))
        {
            return true;
        }

        point_pos = points.find_next(point_pos);
    }

    return false;
}

void measured_model::save_as_model(const model_points_set_t& points,
                                   const formulas_t& contacts_F,
                                   const points_t& all_valid_points)
{
    // TODO this might be done different. But that way it's maybe most readable.

    points_ = get_points_from_subset(points, all_valid_points);
    variable_evaluations_ = generate_variable_evaluations(points_, used_variables_.size());
    contact_relations_ = build_contact_relations_matrix(contacts_F, variable_evaluations_);

    // The variables in the system are all valid points. Restrict them only to the subset of points.
    // It is slower to create different system every time(allocations, remapping, etc...)
    term_to_modal_points_t term_evaluations;
    add_term_evaluations(term_evaluations, measured_less_eq_F_, variable_evaluations_);
    add_term_evaluations(term_evaluations, measured_less_eq_T_, variable_evaluations_);

    auto system = system_of_inequalities(points_.size());
    const auto points_set = ~model_points_set_t(points_.size());
    const auto res = create_system_of_inequalities(points_set, term_evaluations, system);
    assert(res);

    system_ = std::move(system);
}

auto measured_model::create_system_of_inequalities(const model_points_set_t& points,
                                                   const term_to_modal_points_t& term_evaluations,
                                                   system_of_inequalities& system) const -> bool
{
    // For each <=m(a,b) calculate v(a) and v(b), then we will create
    // an inequality of the following type: SUM_I Xi <= SUM_J Xj, where I is v(a) and J is v(b).
    // For each ~<=m(a,b) calculate v(a) and v(b), then we will create
    // an inequality of the following type: SUM_I Xi > SUM_J Xj, where I is v(a) and J is v(b).
    // Each inequality is a row in the system of inequalities. If this system has a solution, then we are good.
    system.clear();

    auto add_constraint = [](auto& s, const auto& formula, const auto& points, const auto& term_evaluations, const auto& op) -> bool
    {
        const auto a = formula->get_left_child_term();
        const auto b = formula->get_right_child_term();
        const auto eval_it_a = term_evaluations.find(a);
        const auto eval_it_b = term_evaluations.find(b);
        assert(eval_it_a != term_evaluations.end());
        assert(eval_it_b != term_evaluations.end());

        // The evaluations contains all valid modal points, so restrict them to the provided subset @points.
        const auto eval_a = eval_it_a->second & points;
        const auto eval_b = eval_it_b->second & points;

        return s.add_constraint(eval_a, eval_b, op);
    };

    for(const auto& m : measured_less_eq_T_)
    {
        if(!add_constraint(system, m, points, term_evaluations, system_of_inequalities::inequality_operation::LE))
        {
            return false;
        }
    }

    for(const auto& m : measured_less_eq_F_)
    {
        if(!add_constraint(system, m, points, term_evaluations, system_of_inequalities::inequality_operation::G))
        {
            return false;
        }
    }

    return system.is_solvable();
}

void measured_model::clear()
{
    used_variables_.clear();
    measured_less_eq_T_.clear();
    measured_less_eq_F_.clear();
    system_.clear();

    processed_combinations_ = {};

    imodel::clear();
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

    const auto unset_common_variables = [](auto& v_a, auto& v_b)
    {
        const auto orig_v_a = v_a;
        v_a = v_a & ~v_b;
        v_b = v_b & ~orig_v_a;
    };

    out << "System:\n";
    for(const auto& m : measured_less_eq_T_) // <=m(a,b)
    {
        auto v_a = m->get_left_child_term()->evaluate(variable_evaluations_, points_size);
        auto v_b = m->get_right_child_term()->evaluate(variable_evaluations_, points_size);

        unset_common_variables(v_a, v_b);

        out << "| ";
        print_system_sum_variables(out, v_a);
        out << " <= ";
        print_system_sum_variables(out, v_b);
        out << " # " << *m << "\n";
    }

    for(const auto& m : measured_less_eq_F_) // ~<=m(a,b)
    {
        auto v_a = m->get_left_child_term()->evaluate(variable_evaluations_, points_size);
        auto v_b = m->get_right_child_term()->evaluate(variable_evaluations_, points_size);

        unset_common_variables(v_a, v_b);

        out << "| ";
        print_system_sum_variables(out, v_a);
        out << " > ";
        print_system_sum_variables(out, v_b);
        out << " # ~" << *m << "\n";
    }

    for(size_t i = 0; i < points_size; ++i) // all unknowns should be positive (i.e. each variable unknown > 0)
    {
        out << "| ";
        out << "X" << i << " > 0 # Positive restriction\n";
    }

    return out;
}

auto measured_model::get_modal_points_measured_values() const -> std::vector<double>
{
    if(!system_.is_solvable())
    {
        return {};
    }

    return system_.get_variables_values();
}

auto measured_model::print(std::ostream& out) const -> std::ostream&
{
    if(measured_less_eq_T_.empty() && measured_less_eq_F_.empty())
    {
        out << "No system needed. No measured atomic formulas:\n";
        return out;
    }

    print_system(out);

    if(system_.is_solvable())
    {
        constexpr auto max_precision {std::numeric_limits<double>::digits10 + 1};

        out << "Solution:\n";
        auto variables_values = system_.get_variables_values();
        assert(variables_values.size() == points_.size());

        for(size_t i = 0; i < variables_values.size(); ++i)
        {
            out << "m(" << i << ") = X" << i << " = " << std::fixed << std::setprecision(max_precision) << variables_values[i] << "\n";
        }
    }
    else
    {
        out << "No solution!\n";
    }

    return out;
}
