#include "variables_evaluations_block.h"

#include <cassert>

variables_evaluations_block::variables_evaluations_block(const variables_mask_t& variables)
    : variables_A_(variables)
{
    variables_B_ = variables_mask_t(variables_A_.size()); // empty subset
    init();
}

variables_evaluations_block::variables_evaluations_block(const variables_mask_t& variables_A, const variables_mask_t& variables_B)
    : variables_A_(variables_A)
    , variables_B_(variables_B)
{
    init();
}

auto variables_evaluations_block::operator==(const variables_evaluations_block& rhs) const -> bool
{
    return variables_A_ == rhs.variables_A_ &&
           variables_B_ == rhs.variables_B_ &&
           evaluations_ == rhs.evaluations_;
}

auto variables_evaluations_block::operator!=(const variables_evaluations_block& rhs) const -> bool
{
    return !operator==(rhs);
}

auto variables_evaluations_block::get_variables() const -> variables_mask_t
{
    return get_variables_A() | get_variables_B();
}

auto variables_evaluations_block::get_variables_A() const -> const variables_mask_t&
{
    return variables_A_;
}

auto variables_evaluations_block::get_variables_B() const -> const variables_mask_t&
{
    return variables_B_;
}

auto variables_evaluations_block::get_set_variables_ids_A() const -> const set_variables_ids_t&
{
    return set_variables_ids_A_;
}

auto variables_evaluations_block::get_set_variables_ids_B() const -> const set_variables_ids_t&
{
    return set_variables_ids_B_;
}

auto variables_evaluations_block::get_evaluations() -> variables_evaluations_t&
{
    return evaluations_;
}

auto variables_evaluations_block::get_evaluations() const -> const variables_evaluations_t&
{
    return evaluations_;
}

auto variables_evaluations_block::generate_next_evaluation_over_A() -> bool
{
    return generate_next_evaluation(variables_A_, set_variables_ids_A_);
}

auto variables_evaluations_block::generate_next_evaluation_over_B() -> bool
{
    return generate_next_evaluation(variables_B_, set_variables_ids_B_);
}

void variables_evaluations_block::reset_evaluations()
{
    reset_evaluations_of_A();
    reset_evaluations_of_B();
}

void variables_evaluations_block::reset_evaluations_of_A()
{
    reset_evaluations(variables_A_);
}

void variables_evaluations_block::reset_evaluations_of_B()
{
    reset_evaluations(variables_B_);
}

void variables_evaluations_block::init()
{
    assert(variables_A_.size() == variables_B_.size());
    assert((variables_A_ & variables_B_).none());

    const auto number_of_variables = variables_A_.size();
    if(!number_of_variables)
    {
        return;
    }

    evaluations_.resize(number_of_variables); // the first evaluation is from 0s.

    // Fill the indexes of the set bits in reverse order, i.e.
    // write the ids of the set variables in the mask
    set_variables_ids_A_.reserve(variables_A_.count());
    set_variables_ids_B_.reserve(variables_B_.count());

    auto i = number_of_variables;
    while(i-- > 0)
    {
        if(variables_A_.test(i))
        {
            set_variables_ids_A_.push_back(i);
        }

        if(variables_B_.test(i))
        {
            set_variables_ids_B_.push_back(i);
        }
    }
}

auto variables_evaluations_block::generate_next_evaluation(const variables_mask_t& variables, const set_variables_ids_t& set_variables_ids) -> bool
{
    if((variables & evaluations_) == variables)
    {
        // If the evaluation for the variables is only 1s then we cannot generate a new one,
        // i.e. we have already generated all of them.
        return false;
    }

    /*
     * Will generate the evaluations in the following order: 0...00, 0...01, 0...10, ... , 11...10, 11...11.
     * This is very similar to the increment(+1) operation of integer numbers in their binary representation.
     * For the binary number an algorithm could be the following:
     * Iterate all bits starting from the least significant. If there are N bits: i = 0...n-1
     *     - bit(i) == 1 => bit(i) = 0
     *     - bit(i) == 0 => bit(i) = 1 & stop
     * In our case it is similar, we want to make the increment operation only on the set bits in the
     * variables_ mask.
     * set_variables_ids_ has the ids of the set bits in the variables mask in reverce order
     */
    for(const auto id : set_variables_ids)
    {
        if(!evaluations_[id])
        {
            evaluations_.set(id);
            break;
        }
        else
        {
            evaluations_.set(id, false);
        }
    }

    return true;
}

void variables_evaluations_block::reset_evaluations(const variables_mask_t& variables)
{
    evaluations_ = evaluations_ & ~variables;
}
