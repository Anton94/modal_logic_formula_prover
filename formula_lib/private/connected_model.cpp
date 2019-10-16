#include "connected_model.h"
#include "formula.h"
#include "formula_mgr.h"
#include "term.h"

#include <cassert>
#include <queue>

auto connected_model::create(const formulas_t& contacts_T, const formulas_t& contacts_F,
                             const terms_t& zero_terms_T, const terms_t& zero_terms_F,
                             const variables_mask_t& used_variables, const formula_mgr* mgr) -> bool
{
    clear();
    used_variables_ = used_variables;
    mgr_ = mgr;

    construct_all_valid_unique_points(contacts_F, zero_terms_T);

    calculate_the_model_evaluation_of_each_variable();

    if(!is_zero_terms_F_rule_satisfied(zero_terms_F) ||
       !is_contacts_T_existence_rule_satisfied(contacts_T))
    {
        trace() << "Unable to create model points even to satisfy the existence of points in the model evaluation of != 0 and contact terms.";
        return false;
    }

    if(!build_contact_relations_matrix(contacts_T, contacts_F))
    {
        trace() << "Unable to create contact relations which satisfies ~C and C atomic formulas.";
        return false;
    }

    // Now, we have in some sence the biggest model(w.r.t number of unique points and maximal contact relations between them)
    assert(mgr_->is_model_satisfiable(*this));

    const auto connected_components = get_connected_components();

    const auto original_variable_evaluations = variable_evaluations_; // TODO: consider modifying the checking if the component is a model instead of modifying the variable_evaluations_
    for(const auto& connected_component : connected_components)
    {
        // Restrict the variable evaluations to only those points which are in the @connected_component
        reduce_variable_evaluations_to_subset_of_points(connected_component);

        if(is_zero_terms_F_rule_satisfied(zero_terms_F) &&
           is_contacts_T_rule_satisfied(contacts_T))
        {
            // Good. The connected component is also a valid model.
            // Remove all other points(outside the connected component) because we do not need them.
            reduce_model_to_subset_of_points(connected_component);
            assert(mgr_->is_model_satisfiable(*this));
            return true;
        }

        // rollback the variable_evaluations
        variable_evaluations_ = original_variable_evaluations;
    }

    return false;
}

void connected_model::construct_all_valid_unique_points(const formulas_t& contacts_F,
                                                        const terms_t& zero_terms_T)
{
    variables_evaluations_block evaluation(used_variables_);

    do
    {
        if(are_zero_terms_T_satisfied(zero_terms_T, evaluation) &&
           is_contacts_F_rule_satisfied_only_reflexivity(contacts_F, evaluation))
        {
            points_.push_back(evaluation);
        }
    } while(evaluation.generate_next_evaluation());
}

auto connected_model::get_model_points() const -> const points_t&
{
    return points_;
}

void connected_model::clear()
{
    used_variables_.clear();
    points_.clear();

    imodel::clear();
}

auto connected_model::are_zero_terms_T_satisfied(const terms_t& zero_terms_T,
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

auto connected_model::is_contacts_F_rule_satisfied_only_reflexivity(
    const formulas_t& contacts_F, const variables_evaluations_block& evaluation) const -> bool
{
    for(const auto& c : contacts_F)
    {
        const auto a = c->get_left_child_term();
        const auto b = c->get_right_child_term();

        if(a->evaluate(evaluation).is_constant_true() && b->evaluate(evaluation).is_constant_true())
        {
            return false;
        }
    }
    return true;
}

auto connected_model::is_zero_terms_F_rule_satisfied(const terms_t& zero_terms_F) const -> bool
{
    for(const auto& z : zero_terms_F)
    {
        if(z->evaluate(variable_evaluations_, points_.size()).none())
        {
            return false;
        }
    }
    return true;
}

auto connected_model::is_contacts_T_existence_rule_satisfied(const formulas_t& contacts_T) const -> bool
{
    for(const auto& c : contacts_T)
    {
        const auto a = c->get_left_child_term();
        const auto b = c->get_right_child_term();

        if(a->evaluate(variable_evaluations_, points_.size()).none() ||
           b->evaluate(variable_evaluations_, points_.size()).none())
        {
            return false;
        }
    }
    return true;
}

auto connected_model::is_contacts_T_rule_satisfied(const formulas_t& contacts_T) const -> bool
{
    // C(a,b)
    for(const auto& c : contacts_T)
    {
        const auto v_a = c->get_left_child_term()->evaluate(variable_evaluations_, points_.size());
        const auto v_b = c->get_left_child_term()->evaluate(variable_evaluations_, points_.size());

        auto point_from_v_a = v_a.find_first();
        while (point_from_v_a != model_points_set_t::npos)
        {
            const auto& contacts_of_point_from_v_a = contact_relations_[point_from_v_a];
            if ((contacts_of_point_from_v_a & v_b).none())
            {
                return false;
            }

            point_from_v_a = v_a.find_next(point_from_v_a);
        }
    }
    return true;
}

auto connected_model::build_contact_relations_matrix(const formulas_t& contacts_T, const formulas_t& contacts_F) -> bool
{
    const auto point_size = points_.size();
    contact_relations_.clear();
    contact_relations_.resize(point_size, model_points_set_t(point_size, true)); // Fill NxN matrix with 1s

    // Removes the connection between each point in v_l and all points from v_r
    auto connection_remover = [&](auto& v_l, auto& v_r)
    {
        const auto neg_v_r = ~v_r;

        auto point_from_v_l = v_l.find_first();
        while (point_from_v_l != model_points_set_t::npos)
        {
            auto& contacts_of_point_from_v_l = contact_relations_[point_from_v_l];
            contacts_of_point_from_v_l &= neg_v_r; // remove all points in @contacts_of_point_from_v_l which are also in @v_r

            point_from_v_l = v_l.find_next(point_from_v_l);
        }
    };

    // ~C(a,b)
    for(const auto& c : contacts_F)
    {
        const auto v_a = c->get_left_child_term()->evaluate(variable_evaluations_, points_.size());
        const auto v_b = c->get_left_child_term()->evaluate(variable_evaluations_, points_.size());

        assert((v_a & v_b).none()); // no common points, because we built them that way

        connection_remover(v_a, v_b);
        connection_remover(v_b, v_a);
    }

    return is_contacts_T_rule_satisfied(contacts_T);
}

auto connected_model::get_connected_components() const -> std::vector<model_points_set_t>
{
    if(points_.empty())
    {
        return {};
    }

    std::vector<model_points_set_t> connected_components;
    model_points_set_t not_visited_points(points_.size(), true); // bitset of 1s for the not visited points, inverted because we have fast finding of 1s in the bitset

    while(not_visited_points.any())
    {
        size_t root_point_id = not_visited_points.find_first();

        auto connected_component = get_connected_component(root_point_id, not_visited_points);
        connected_components.push_back(std::move(connected_component));
    }
    return connected_components;
}

auto connected_model::get_connected_component(size_t root_point_id, model_points_set_t& not_visited_points) const -> model_points_set_t
{
    assert(not_visited_points.test_set(root_point_id));

    model_points_set_t connected_component(points_.size());

    // A simple BFS traversing
    std::queue<size_t> q;
    q.push(root_point_id);

    while(!q.empty())
    {
        const auto point_id = q.front();
        q.pop();
        assert(not_visited_points.test_set(point_id));

        connected_component.set(point_id);
        not_visited_points.set(point_id, false);

        const auto& point_connections = contact_relations_[point_id];
        auto connected_point_id = point_connections.find_first();
        while(connected_point_id != model_points_set_t::npos)
        {
            if(not_visited_points.test_set(connected_point_id))
            {
                q.push(connected_point_id);
            }
            connected_point_id = point_connections.find_next(connected_point_id);
        }
    }

    return connected_component;
}

void connected_model::reduce_variable_evaluations_to_subset_of_points(const model_points_set_t& points_subset)
{
    for(auto& evaluation : variable_evaluations_)
    {
        evaluation &= points_subset;
    }
}

void connected_model::reduce_model_to_subset_of_points(const model_points_set_t& points_subset)
{
    points_t reduced_points;
    const auto reduced_points_size = points_subset.count();
    reduced_points.reserve(reduced_points_size);
    std::unordered_map<size_t, size_t> point_id_old_to_new;
    point_id_old_to_new.reserve(reduced_points_size);

    auto point = points_subset.find_first();
    while (point != model_points_set_t::npos)
    {
        point_id_old_to_new[point] = reduced_points.size();

        reduced_points.push_back(points_[point]);
        point = points_subset.find_next(point);
    }

    // TODO: example, explaining, etc.
    auto map_subset_of_points_to_reduced_points = [&](const model_points_set_t& points_subset) -> model_points_set_t
    {
        model_points_set_t mapped_subset(reduced_points_size); // all 0s

        auto point = points_subset.find_first();
        while (point != model_points_set_t::npos)
        {
            mapped_subset.set(point_id_old_to_new[point]);
            point = points_subset.find_next(point);
        }
        return mapped_subset;
    };

    contacts_t reduced_contact_relations(reduced_points_size, model_points_set_t(reduced_points_size)); // KxK matrix of 0s (K=@reduced_points_size)

    point = points_subset.find_first();
    while (point != model_points_set_t::npos)
    {
        auto& point_contacts = contact_relations_[point];
        point_contacts &= points_subset; // remove extra contacts

        const auto reduced_point_id = point_id_old_to_new[point];
        reduced_contact_relations[reduced_point_id] = map_subset_of_points_to_reduced_points(point_contacts);
        point = points_subset.find_next(point);
    }

    points_ = std::move(reduced_points);
    contact_relations_ = std::move(reduced_contact_relations);
}

// TODO: common function
void connected_model::calculate_the_model_evaluation_of_each_variable()
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

auto connected_model::print(std::ostream& out) const -> std::ostream&
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
