#include "evaluator.h"
#include "logger.h"

namespace evaluator
{

namespace
{

/*
 *
 * Think of the map of evaluations as an array of true(1) and false(0) values, e.g. [0, 0, 0].
 * Will generate all combinations of 0/1 (total - 2^n where n is the number of variables). After each
 * generation will check if it satisfies the formula.
 * The generation should be in the following order:
 * [0, 0, 0], [0, 0, 1], [0, 1, 0], [0, 1, 1], [1, 0, 0], [1, 0, 1], [1, 1, 0], [1, 1, 1]
 *
 * Think of the array as a head (the element pointer by @it) and a tail (all elements after the head).
 * For our example ([0, 0, 0]) and in the first call when @it points to the first element, the head is 0 and
 * the tail is [0, 0].
 * Imagine that has_satisfiable_evaluation works for a smaller set of variables(the elements in the tail).
 * To generate all combinations we can set the head element to 0 and generate (recursively) all combinations
 * in the tail,
 * i.e. call again has_satisfiable_evaluation but move the head to the next element. Then set the head element
 * to 1 and generate all combinations in the tail.
 *
*/
auto has_satisfiable_evaluation(const formula& f, variable_evaluations_t& evaluations,
                                variable_evaluations_t::iterator it) -> bool
{
    if(it == evaluations.end())
    {
        auto res = f.evaluate(evaluations);
        info() << "Evaluation with " << evaluations << " " << (res ? "succeeded" : "failed");
        return res;
    }

    auto next = it;
    ++next;

    it->second = false;
    if(has_satisfiable_evaluation(f, evaluations, next))
    {
        return true;
    }

    it->second = true;
    return has_satisfiable_evaluation(f, evaluations, next);
}
}

auto has_satisfiable_evaluation(const formula& f) -> bool
{
    info() << "Running satisfiable evalution checking of " << f;
    variables_t variables;
    f.get_variables(variables);

    variable_evaluations_t evaluations;
    for(const auto& variable : variables)
    {
        evaluations[variable] = false;
    }

    return has_satisfiable_evaluation(f, evaluations, evaluations.begin());
}
}
