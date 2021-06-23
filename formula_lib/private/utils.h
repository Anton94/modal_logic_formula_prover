#pragma once

#include <chrono>
#include <functional>
#include <vector>

#include "types.h"
#include "variables_evaluations_block.h"

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

using time_type = std::chrono::milliseconds;

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
