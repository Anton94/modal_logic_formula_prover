#pragma once

#include "../private/types.h"

class variables_evaluations_block
{
    /*
     * The idea is to keep the information of which variables are participating in that evaluation in a bitset, i.e. a mask.
     * A set bit at index X means that the variable with id X participates in the evaluation block.
     * The evaluations are also kept in a bitset manner. If there is a set bit in the variables mask at position X that means
     * that the corresponding bit(at position X) in evaluations_ is the evaluation for the variable with id X.
     *
     */
public:
    variables_evaluations_block(const variables_mask_t& variables);

    variables_evaluations_block(const variables_evaluations_block&) = delete;
    variables_evaluations_block& operator=(const variables_evaluations_block&) = delete;
    variables_evaluations_block(variables_evaluations_block&&) = default;
    variables_evaluations_block& operator=(variables_evaluations_block&&) = default;

    auto get_variables() const -> const variables_mask_t&;
    auto get_evaluations() const -> variables_evaluations_t;

    using set_variables_ids_t = std::vector<variable_id_t>;
    auto get_set_variables_ids() const -> const set_variables_ids_t&;

    auto generate_next_evaluation() -> bool;

private:
    const variables_mask_t variables_;
    variables_evaluations_t evaluations_;

    set_variables_ids_t set_variables_ids_; // for generating the next evaluations in order to make it O(|set varaibles|) instead of O(|all variables in the mask|)
};
