#include "variables_evaluations_block_stack.h"
#include "logger.h"

variables_evaluations_block_stack::variables_evaluations_block_stack(size_t block_size)
    : combined_variables_(block_size, false)
    , combined_evaluations_(block_size, false)
{
}

auto variables_evaluations_block_stack::top() const -> const variables_evaluations_block&
{
    assert(!block_stack_.empty());
    return block_stack_.top();
}

void variables_evaluations_block_stack::push(variables_evaluations_block&& block)
{
    block_stack_.push(std::move(block));
    update_combined_after_push();
}

void variables_evaluations_block_stack::push(const variables_evaluations_block& block)
{
    block_stack_.push(block);
    update_combined_after_push();
}

void variables_evaluations_block_stack::pop()
{
    assert(!block_stack_.empty());
    update_combined_before_pop();
    block_stack_.pop();
}

auto variables_evaluations_block_stack::empty() const -> bool
{
    return block_stack_.empty();
}

auto variables_evaluations_block_stack::get_combined_block() const -> variables_evaluations_block
{
    // TODO: do not create it each time!
    variables_evaluations_block combined(combined_variables_);
    combined.get_evaluations() = combined_evaluations_;
    return combined;
}

auto variables_evaluations_block_stack::get_combined_variables() const -> const variables_mask_t&
{
    return combined_variables_;
}

auto variables_evaluations_block_stack::get_combined_evaluations() const -> const variables_evaluations_t&
{
    return combined_evaluations_;
}

auto variables_evaluations_block_stack::generate_evaluation() -> bool
{
    if(empty())
    {
        return false;
    }

    auto& top_block = block_stack_.top();

    if(!top_block.generate_next_evaluation_over_A())
    {
        return false;
    }

    /*
    The idea:
        First: unset the bits from the old_top_block_evaluation
        Then:  add the new evaluations to the combined evaluation

    For example:
    combined_variables_      = 1 1 1 1 1 0
    combined_evaluations_    = 0 0 1 0 1 0
    top_block    vars        = 0 1 1 0 0 0
    top_block    evals       = 0 0 1 0 0 0
    ----------------------------------------unset the bits from the old_top_block_evaluation->
    -> combined_evaluations_ = 0 0 1 0 1 0
    top_block new evals      = 0 1 0 0 0 0
    ----------------------------------------add the new evals to the combined evaluation->
    -> combined_evaluations_ = 0 0 0 0 1 0
    */
    const auto combined_vars_without_top_block_vars = combined_variables_ & ~top_block.get_variables_A();
    combined_evaluations_ &= combined_vars_without_top_block_vars;

    combined_evaluations_ |= top_block.get_evaluations();

    assert(combined_variables_.size() == combined_evaluations_.size());
    return true;
}

void variables_evaluations_block_stack::update_combined_after_push()
{
    assert(!block_stack_.empty());
    const auto& pushed_block = block_stack_.top();
    if((combined_evaluations_ & pushed_block.get_variables_A()).any())
    {
        error()
            << "Adding variables evaluations block with set variable ids which are alredy there. Ambiguous.";
        pop();
    }

    if(pushed_block.get_variables_A().size() != combined_variables_.size())
    {
        error() << "Adding a block with different size.";
        pop();
    }

    combined_variables_ |= pushed_block.get_variables_A();
    combined_evaluations_ |= pushed_block.get_evaluations();

    assert(combined_variables_.size() == combined_evaluations_.size());
}

void variables_evaluations_block_stack::update_combined_before_pop()
{
    assert(!block_stack_.empty());
    const auto& block_to_pop = block_stack_.top();

    /*
        The idea:
            First: unset the bits from the block_to_pop's variables
            Then:  unset the 1s from block_to_pop's evaluation(which are also in the combined evaluation),
               the 0s are not effecting it at all,

        For example:
        combined_variables_      = 1 1 1 1 1 0
        combined_evaluations_    = 0 0 1 0 1 0
        block_to_pop vars        = 0 1 1 0 0 0
        block_to_pop evals       = 0 0 1 0 0 0
        ----------------------------------------unset the bits from the block_to_pop's variables->
        -> combined_variables_   = 1 0 0 1 1 0
           combined_evaluations_ = 0 0 1 0 1 0
        ----------------------------------------unset the 1s from block_to_pop's evaluation->
           combined_variables_   = 1 0 0 1 1 0
        -> combined_evaluations_ = 0 0 0 0 1 0
    */
    combined_variables_ &= ~block_to_pop.get_variables_A();
    combined_evaluations_ &= combined_variables_;

    assert(combined_variables_.size() == combined_evaluations_.size());
}
