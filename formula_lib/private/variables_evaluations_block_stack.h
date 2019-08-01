#pragma once

#include "types.h"
#include "variables_evaluations_block.h"

#include <stack>

class variables_evaluations_block_stack
{
    /*
    * The purpose of this stack is to hold many variables_evaluations_blocks and to provide a combined
    * evaluation/variables_mask of all of them.
    * The oprations of adding a new block(push), generating a new evalution of the top one, poping a block
    * should be constant.
    * All blocks should have the same size specified in the ctor
    */
public:
    variables_evaluations_block_stack(size_t block_size = 0);

    variables_evaluations_block_stack(const variables_evaluations_block_stack&) = delete;
    variables_evaluations_block_stack& operator=(const variables_evaluations_block_stack&) = delete;
    variables_evaluations_block_stack(variables_evaluations_block_stack&&) = default;
    variables_evaluations_block_stack& operator=(variables_evaluations_block_stack&&) = default;

    auto top() const -> const variables_evaluations_block&;
    void push(variables_evaluations_block&& block);
    void push(const variables_evaluations_block& block);
    void pop();
    auto empty() const -> bool;

    auto get_combined_block() const -> variables_evaluations_block;
    auto get_combined_variables() const -> const variables_mask_t&;
    auto get_combined_evaluations() const -> const variables_evaluations_t&;

    // Tries to generate a new evaluation of the top block.
    auto generate_evaluation() -> bool;

private:
    void update_combined_after_push();
    void update_combined_before_pop();

    size_t block_size_;
    std::stack<variables_evaluations_block> block_stack_;

    variables_mask_t combined_variables_;
    variables_evaluations_t combined_evaluations_;
};
