#pragma once

#include "types.h"

class variables_evaluations_block
{
    /*
     * The idea is to keep the information of which variables are participating in that evaluation in a
     * bitset, i.e. a mask.
     * A set bit at index X means that the variable with id X participates in the evaluation block.
     * The evaluations are also kept in a bitset manner. If there is a set bit in the variables mask at
     * position X that means
     * that the corresponding bit(at position X) in evaluations_ is the evaluation for the variable with id X.
     *
     */
public:
    variables_evaluations_block(const variables_mask_t& variables_A);
    /// variables_A and variables_B does not have common elements and should have same sizes!
    variables_evaluations_block(const variables_mask_t& variables_A, const variables_mask_t& variables_B);

    variables_evaluations_block(const variables_evaluations_block&) = default;
    variables_evaluations_block& operator=(const variables_evaluations_block&) = default;
    variables_evaluations_block(variables_evaluations_block&&) = default;
    variables_evaluations_block& operator=(variables_evaluations_block&&) = default;

    auto operator==(const variables_evaluations_block& rhs) const -> bool;
    auto operator!=(const variables_evaluations_block& rhs) const -> bool;

    auto get_variables() const -> variables_mask_t;
    auto get_variables_A() const -> const variables_mask_t&;
    auto get_variables_B() const -> const variables_mask_t&;
    auto get_evaluations() -> variables_evaluations_t&;
    auto get_evaluations() const -> const variables_evaluations_t&;

    using set_variables_ids_t = std::vector<variable_id_t>;
    auto get_set_variables_ids_A() const -> const set_variables_ids_t&;
    auto get_set_variables_ids_B() const -> const set_variables_ids_t&;

    auto generate_next_evaluation_over_A() -> bool;
    auto generate_next_evaluation_over_B() -> bool;

    void reset_evaluations();
    void reset_evaluations_of_A();
    void reset_evaluations_of_B();

private:
    void init();
    auto generate_next_evaluation(const variables_mask_t& variables, const set_variables_ids_t& set_variables_ids) -> bool;
    void reset_evaluations(const variables_mask_t& variables);

    variables_mask_t variables_A_;
    variables_mask_t variables_B_;
    variables_evaluations_t evaluations_;

    set_variables_ids_t set_variables_ids_A_; // for generating the next evaluations in order to make it
                                            // O(|set varaibles|) instead of O(|all variables in the mask|)
    set_variables_ids_t set_variables_ids_B_;
};
