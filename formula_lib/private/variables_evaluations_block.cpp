#include "variables_evaluations_block.h"

variables_evaluations_block::variables_evaluations_block(const variables_mask_t& variables)
    : variables_(variables)
{
    if(variables_.empty())
    {
        return;
    }

    const auto number_of_variables = variables_.size();
    evaluations_.resize(number_of_variables); // the first evaluation is from 0s.

    // Fill the indexes of the set bits in reverse order, i.e.
    // write the ids of the set variables in the mask
    set_variables_ids_.reserve(variables_.count());
    auto i = number_of_variables;
    while(i-- > 0)
    {
        if(variables_[i])
        {
            set_variables_ids_.push_back(i);
        }
    }
}

auto variables_evaluations_block::operator==(const variables_evaluations_block& rhs) const -> bool
{
    return variables_ == rhs.variables_ && evaluations_ == rhs.evaluations_ &&
           set_variables_ids_ == rhs.set_variables_ids_;
}

auto variables_evaluations_block::operator!=(const variables_evaluations_block& rhs) const -> bool
{
    return !operator==(rhs);
}

auto variables_evaluations_block::get_variables() const -> const variables_mask_t&
{
    return variables_;
}

auto variables_evaluations_block::get_set_variables_ids() const -> const set_variables_ids_t&
{
    return set_variables_ids_;
}

auto variables_evaluations_block::get_evaluations() const -> variables_evaluations_t
{
    return evaluations_;
}

auto variables_evaluations_block::generate_next_evaluation() -> bool
{
    if((variables_ & evaluations_) == variables_)
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
     * variables mask.
     * set_variables_ids_ has the ids of the set bits in the variables mask in reverce order
     */
    for(const auto id : set_variables_ids_)
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
