#include "evaluator.h"
#include "logger.h"

namespace evaluator
{

namespace
{

/*
 * Will generate all possible combinations for variable evaluations and check with each one of them
 * until a satisfiable one is found. 2^n combinations for n variables.
 *
 * The idea is simple. Think of the map of evaluations as an array of true(1) and false(0) values, e.g. [0, 0, 0].
 * In the first call, @it will point to the first element (@it points to the head) and we will think the elements after the one pointer by @it as a tail.
 * Take the head element (pointed by @it), set it to false(0), iterate over the tail and make all combinations of 0/1, i.e.
 * call again has_satisfiable_evaluation but move the head to the next element.
 * When all elements has been iterated we are checking if the current combination state is leading us to a satisfiable function(the botom of the recursion).
 * If it does not lead us to success, we change the head element to true and iterate over the tail and make all combinations of 0/1, etc.
 *
 * It should try to evaluate f with the following variable evaluations(for the example with three variables):
 * [0, 0, 0], [0, 0, 1], [0, 1, 0], [0, 1, 1], [1, 0, 0], [1, 0, 1], [1, 1, 0], [1, 1, 1]
 *
*/
auto has_satisfiable_evaluation(const formula& f, variable_evaluations_t& evaluations, variable_evaluations_t::iterator it) -> bool
{
    if (it == evaluations.end())
    {
        auto res = f.evaluate(evaluations);
        info() << "Evaluation with " << evaluations << " " << (res ? "succeeded" : "failed");
        return res;
    }

    auto next = it;
    ++next;

    it->second = false;
    if (has_satisfiable_evaluation(f, evaluations, next))
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
    for (const auto& variable : variables)
    {
        evaluations[variable] = false;
    }

    return has_satisfiable_evaluation(f, evaluations, evaluations.begin());
}

}
