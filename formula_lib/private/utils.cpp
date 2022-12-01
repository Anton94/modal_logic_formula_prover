#include "utils.h"
#include "formula.h"
#include "term.h"
#include "thread_termiator.h"

#include <numeric>

call_on_destroy::call_on_destroy(std::function<void()>&& callback)
    : callback_(std::move(callback))
{
}

call_on_destroy::~call_on_destroy()
{
    if(callback_)
    {
        callback_();
    }
}

uint64_t to_int(const std::vector<bool>& v)
{
    return std::accumulate(v.begin(), v.end(), 0ull, [](uint64_t acc, bool bit) { return (acc << 1) + bit; });
}

auto are_zero_terms_T_satisfied(const terms_t& zero_terms_T,
                                const variables_evaluations_block& evaluation) -> bool
{
    // The evaluation should evaluate all zero terms to constant false.
    // That way it will not participate in any of their evaluations.
    for(const auto& z : zero_terms_T)
    {
        if(!z->evaluate(evaluation).is_constant_false())
        {
            return false;
        }
    }
    return true;
}

auto is_contacts_F_reflexive_rule_satisfied(const formulas_t& contacts_F,
                                            const variables_evaluations_block& evaluation) -> bool
{
    for(const auto& c : contacts_F)
    {
        // The evaluation should not be part of both non-contact term's evaluations.
        const auto left_t = c->get_left_child_term();
        const auto right_t = c->get_right_child_term();
        if(left_t->evaluate(evaluation).is_constant_true() &&
           right_t->evaluate(evaluation).is_constant_true())
        {
            return false;
        }
    }
    return true;
}

auto is_contacts_F_connectivity_rule_satisfied(const formulas_t& contacts_F,
                                               const variables_evaluations_block& eval_a,
                                               const variables_evaluations_block& eval_b) -> bool
{
    for(const auto& c : contacts_F)
    {
        // In order the eval_a and eval_b to not conflict with a non-contact
        // they should not participate in the non-contact term's evaluations.
        // In other words, both evaluations should not evaluate both terms to true.
        const auto l = c->get_left_child_term();
        const auto r = c->get_right_child_term();
        if((l->evaluate(eval_a).is_constant_true() &&
            r->evaluate(eval_b).is_constant_true()) ||
           (l->evaluate(eval_b).is_constant_true() &&
            r->evaluate(eval_a).is_constant_true()))
        {
            // The reflexivity case is not taken into account here.
            return false;
        }
    }
    return true;
}

auto construct_all_valid_points(const variables_mask_t& used_variables,
                                const formulas_t& contacts_F,
                                const terms_t& zero_terms_T) -> points_t
{
    const auto expected_points_count = size_t(std::pow(2, used_variables.size()));
    const auto heuristic_points_count = std::min(4096ul, expected_points_count);

    points_t points;
    points.reserve(heuristic_points_count);

    variables_evaluations_block evaluation(used_variables);

    do
    {
        // TERMINATE_IF_NEEDED(); // TODO: not every point but maybe every 100 or something

        if(are_zero_terms_T_satisfied(zero_terms_T, evaluation) &&
           is_contacts_F_reflexive_rule_satisfied(contacts_F, evaluation))
        {
            points.push_back(evaluation);
        }
    } while(evaluation.generate_next_evaluation_over_A());

    return points;
}

auto build_contact_relations_matrix(const formulas_t& contacts_F,
                                    const variable_id_to_points_t& variable_evaluations) -> contacts_t
{
    if(variable_evaluations.empty())
    {
        return {};
    }

    const auto points_count = variable_evaluations.front().size();

    contacts_t contact_relations;
    contact_relations.resize(points_count, ~model_points_set_t(points_count)); // Fill NxN matrix with 1s

    // Removes the connection between each point in v_l and all points from v_r
    auto connection_remover = [&](auto& v_l, auto& v_r)
    {
        const auto neg_v_r = ~v_r;

        auto point_from_v_l = v_l.find_first();
        while (point_from_v_l != model_points_set_t::npos)
        {
            auto& contacts_of_point_from_v_l = contact_relations[point_from_v_l];
            contacts_of_point_from_v_l &= neg_v_r; // remove all points in @contacts_of_point_from_v_l which are also in @v_r

            point_from_v_l = v_l.find_next(point_from_v_l);
        }
    };

    // ~C(a,b)
    for(const auto& c : contacts_F)
    {
        const auto v_a = c->get_left_child_term()->evaluate(variable_evaluations, points_count);
        const auto v_b = c->get_right_child_term()->evaluate(variable_evaluations, points_count);

        assert((v_a & v_b).none()); // no common points, because we built them that way

        connection_remover(v_a, v_b);
        connection_remover(v_b, v_a);
    }

    return contact_relations;
}

auto generate_variable_evaluations(const points_t& points, size_t all_variables_count) -> variable_id_to_points_t
{
    const auto points_size = points.size();
    variable_id_to_points_t variable_evaluations;
    variable_evaluations.resize(all_variables_count, model_points_set_t(points_size)); // initialize each variable evaluation as the empty set

    // Calculate the MODEL evaluation of each variable, i.e. each variable_id
    // v(Pi) = { point | point_evaluation[Pi] == 1 } , i.e. the evaluation of variable with id Pi is 1, i.e. the bit at position Pi is 1
    for (size_t i = 0; i < points_size; ++i)
    {
        const auto& point = points[i];
        const auto& point_evaluation = point.get_evaluations();

        // iterate only set bits(1s)
        auto Pi = point_evaluation.find_first();
        while (Pi != variables_evaluations_t::npos)
        {
            variable_evaluations[Pi].set(i); // adds the point to the v(Pi) set
            Pi = point_evaluation.find_next(Pi);
        }
    }

    return variable_evaluations;
}

void add_term_evaluations(term_to_modal_points_t& term_evaluations, const terms_t& terms, const variable_id_to_points_t& variable_evaluations)
{
    if(variable_evaluations.empty())
    {
        return;
    }

    const auto points_count = variable_evaluations.front().size();

    for(const auto& z : terms)
    {
        term_evaluations[z] = z->evaluate(variable_evaluations, points_count);
    }
}

void add_term_evaluations(term_to_modal_points_t& term_evaluations, const formulas_t& contacts, const variable_id_to_points_t& variable_evaluations)
{
    if(variable_evaluations.empty())
    {
        return;
    }

    const auto points_count = variable_evaluations.front().size();

    for(const auto& c : contacts)
    {
        const auto a = c->get_left_child_term();
        const auto b = c->get_right_child_term();

        term_evaluations[a] = a->evaluate(variable_evaluations, points_count);
        term_evaluations[b] = b->evaluate(variable_evaluations, points_count);
    }
}

auto get_points_from_subset(const model_points_set_t& subset, const points_t& all_points) -> points_t
{
    assert(subset.size() == all_points.size());

    points_t subset_points;
    subset_points.reserve(subset.count());

    auto point_pos = subset.find_first();
    while (point_pos != model_points_set_t::npos)
    {
        const auto& point = all_points[point_pos];
        subset_points.emplace_back(point);

        point_pos = subset.find_next(point_pos);
    }

    return subset_points;
}
