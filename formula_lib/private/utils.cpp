#include "utils.h"
#include "formula.h"
#include "term.h"

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
