#include "variables_evaluations_block_for_positive_term.h"
#include "term.h"

#include <cassert>

variables_evaluations_block_for_positive_term::variables_evaluations_block_for_positive_term(const term& t, const variables_mask_t& variables)
    : term_(std::cref(t))
    , evaluation_block_({})
{
    auto& variables_in_t = t.get_variables();
    assert(variables_in_t.is_subset_of(variables));
    const auto variables_without_thouse_in_t = variables & ~variables_in_t;
    evaluation_block_ = variables_evaluations_block(variables_in_t, variables_without_thouse_in_t);
}

auto variables_evaluations_block_for_positive_term::operator==(const variables_evaluations_block_for_positive_term& rhs) const -> bool
{
    return evaluation_block_ == rhs.evaluation_block_;
}

auto variables_evaluations_block_for_positive_term::operator!=(const variables_evaluations_block_for_positive_term& rhs) const -> bool
{
    return !operator==(rhs);
}

auto variables_evaluations_block_for_positive_term::get_evaluations_block() -> variables_evaluations_block&
{
    return evaluation_block_;
}

auto variables_evaluations_block_for_positive_term::get_evaluations_block() const -> const variables_evaluations_block&
{
    return evaluation_block_;
}

auto variables_evaluations_block_for_positive_term::generate_next_evaluation() -> bool
{
    /*
     * We need to generate a new evaluation which evaluates @t to true.
     * The basic solution is to generate the next evaluation in the sequence and to check if it evaluates @t to true.
     * One optimization is to generate an evaluation for @t which evaluates it to true and fix it, then generate new evaluation only on the variables which are not in @t
     * and when all such evaluations are generated, then we generate a new evaluation over the variables in @t and repeat...
     * We have two separate sets of varaibles, the variables in @t and the rest
     * The variables in @t are the once in evaluation's A set, and the rest are B set.
     * If the current evaluation is true, then we can try to generate next evaluation only on the vars in B (and that will keep the evaluation of @t to be true).
     * Otherwise, we need to generate evaluation on the variables in @t (i.e. in A) which evaluates @t to true and don't forget to reset the evaluation of B
     *
     */
    if(term_.get().evaluate(evaluation_block_).is_constant_true())
    {
        if(evaluation_block_.generate_next_evaluation_over_B())
        {
            return true;
        }
        evaluation_block_.reset_evaluations_of_B();
    }

    while(evaluation_block_.generate_next_evaluation_over_A())
    {
        if(term_.get().evaluate(evaluation_block_).is_constant_true())
        {
            return true;
        }
    }
    return false;
}

auto variables_evaluations_block_for_positive_term::reset() -> bool
{
    evaluation_block_.reset_evaluations();

    // the evaluation of the term should be the constant true, i.e.
    // for t != 0 : the evaluation of 't' with the given @evaluation should be non-zero
    return term_.get().evaluate(get_evaluations_block()).is_constant_true() || generate_next_evaluation();
}
