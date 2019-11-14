#pragma once

#include "types.h"
#include "variables_evaluations_block.h"

class term;

class variables_evaluations_block_for_positive_term
{
    /*
     * See variables_evaluations_block.
     * This is a wrapper which generates only those evaluations which evaluates the passed term @t to constant true.
     *
     */
public:
    /// @variables should be a superset of the variables in @t
    /// NOTE: the passed term @t should be kept alive through the whole life span of @this
    variables_evaluations_block_for_positive_term(const term& t, const variables_mask_t& variables);

    variables_evaluations_block_for_positive_term(const variables_evaluations_block_for_positive_term&) = default;
    variables_evaluations_block_for_positive_term& operator=(const variables_evaluations_block_for_positive_term&) = default;
    variables_evaluations_block_for_positive_term(variables_evaluations_block_for_positive_term&&) = default;
    variables_evaluations_block_for_positive_term& operator=(variables_evaluations_block_for_positive_term&&) = default;

    auto operator==(const variables_evaluations_block_for_positive_term& rhs) const -> bool;
    auto operator!=(const variables_evaluations_block_for_positive_term& rhs) const -> bool;

    auto get_evaluations_block() -> variables_evaluations_block&;
    auto get_evaluations_block() const -> const variables_evaluations_block&;

    /// Generates the next evaluation, which evaluates @t to true
    auto generate_next_evaluation() -> bool;

    /// Resets the evaluation to the 'first' which makes @t to true
    auto reset() -> bool;

private:
    std::reference_wrapper<const term> term_;
    variables_evaluations_block evaluation_block_;
};
