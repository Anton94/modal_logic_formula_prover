#pragma once

#include <chrono>
#include <functional>
#include <vector>

#include "types.h"
#include "variables_evaluations_block.h"

using time_type = std::chrono::milliseconds;
using points_t = std::vector<variables_evaluations_block>;

class call_on_destroy
{
public:
    call_on_destroy(std::function<void()>&& callback);
    ~call_on_destroy();

    call_on_destroy(const call_on_destroy&) = delete;
    call_on_destroy& operator=(const call_on_destroy&) = delete;
    call_on_destroy(call_on_destroy&&) = delete;
    call_on_destroy& operator=(call_on_destroy&&) = delete;

private:
    std::function<void()> callback_;
};

uint64_t to_int(const std::vector<bool>& v);

/// Calls @f, writes the time that took @f to execute in @elapsed_time and returns @f's call result
template <typename R, typename F, typename... Args>
R time_measured_call(F&& f, time_type& elapsed_time, Args&&... args)
{
    const auto t1 = std::chrono::high_resolution_clock::now();
    const auto res = std::forward<F>(f)(std::forward<Args>(args)...);
    const auto t2 = std::chrono::high_resolution_clock::now();
    elapsed_time = std::chrono::duration_cast<time_type>(t2 - t1);
    return res;
}

/// Calls @f and returns the time that took @f to execute
template <typename F, typename... Args>
time_type time_measured_call(F&& f, Args&&... args)
{
    const auto t1 = std::chrono::high_resolution_clock::now();
    std::forward<F>(f)(std::forward<Args>(args)...);
    const auto t2 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<time_type>(t2 - t1);
}

/// Returns true if the evaluation evaluates all zero terms to false.
auto are_zero_terms_T_satisfied(const terms_t& zero_terms_T,
                                const variables_evaluations_block& evaluation) -> bool;

/// Returns whether the evaluation does not conflict with the non-contacts (@contacts_F).
auto is_contacts_F_reflexive_rule_satisfied(const formulas_t& contacts_F,
                                            const variables_evaluations_block& evaluation) -> bool;

/// Returns whether the evaluations does not conflict with the non-contacts (@contacts_F).
auto is_contacts_F_connectivity_rule_satisfied(const formulas_t& contacts_F,
                                               const variables_evaluations_block& eval_a,
                                               const variables_evaluations_block& eval_b) -> bool;

 /// Constructs all model points(evaluations) which are not breaking zero terms and the reflexicity of non-contacts
auto construct_all_valid_points(const variables_mask_t& used_variables,
                                const formulas_t& contacts_F,
                                const terms_t& zero_terms_T) -> points_t;

/// Returns @points_count x @points_count bit matrix with all valid contact relations between all points, which does not interfear with @contacts_F.
auto build_contact_relations_matrix(const formulas_t& contacts_F,
                                    const variable_id_to_points_t& variable_evaluations) -> contacts_t;

/// Generates each variable's evaluation. @all_variables_count are the all variables which are used and the generated evaluation is for all of them.
auto generate_variable_evaluations(const points_t& points, size_t all_variables_count) -> variable_id_to_points_t;

/// Adds each term evaluation to @term_evaluations mapping.
void add_term_evaluations(term_to_modal_points_t& term_evaluations, const terms_t& terms, const variable_id_to_points_t& variable_evaluations);
void add_term_evaluations(term_to_modal_points_t& term_evaluations, const formulas_t& contacts, const variable_id_to_points_t& variable_evaluations);

auto get_points_from_subset(const model_points_set_t& subset, const points_t& all_points) -> points_t;
