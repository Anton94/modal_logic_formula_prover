#include "formula_mgr.h"
#include "logger.h"

namespace
{

using json = nlohmann::json;

auto get_all_variables(const json& f, variables_set_t& variables) -> bool
{
    if(!f.contains("name"))
    {
        error() << "Json (sub)formula is missing a 'name' field:\n" << f.dump(4);
        return false;
    }

    auto& name_field = f["name"];
    if(!name_field.is_string())
    {
        error() << "Json (sub)formula has 'name' field which is not a string:\n" << f.dump(4);
        return false;
    }

    if(!f.contains("value"))
    {
        error() << "Json (sub)formula is missing a 'value' field:\n" << f.dump(4);
        return false;
    }

    auto& value_field = f["value"];
    auto op = name_field.get<std::string>();
    if(op == "string") // variable
    {
        if(!value_field.is_string())
        {
            error() << "Json (sub)formula has 'value' field which is not a string:\n" << f.dump(4);
            return false;
        }
        variables.insert(value_field.get<std::string>());
        return true;
    }

    if(value_field.is_object())
    {
        return get_all_variables(value_field, variables);
    }
    if(value_field.is_array() && value_field.size() == 2)
    {
        return get_all_variables(value_field[0], variables) && get_all_variables(value_field[1], variables);
    }
    error()
        << "Json (sub)formula has 'value' field which is neither an object, nor an array of two objects:\n"
        << f.dump(4);
    return false;
}
}

formula_mgr::formula_mgr()
    : f_(this)
{
}

auto formula_mgr::build(json& f) -> bool
{
    clear();

    // Will cash all variables and swap the string representations with their id in the cache.
    variables_set_t variables_set;
    if(!get_all_variables(f, variables_set))
    {
        return false;
    }

    variables_.reserve(variables_set.size());
    variable_to_id_.reserve(variables_set.size());
    for(const auto& variable : variables_set)
    {
        variable_to_id_[variable] = variables_.size();
        variables_.emplace_back(variable);
    }

    return change_variables_to_variable_ids(f) && f_.build(f);
}

void formula_mgr::get_variables(variables_set_t& out_variables) const
{
    out_variables.insert(variables_.begin(), variables_.end());
}

auto formula_mgr::brute_force_evaluate() const -> bool
{
    info() << "Running brute force evalution checking of " << f_;
    variable_evaluations_bitset_t evaluations(variables_.size(), false);
    return has_satisfiable_evaluation(f_, evaluations, evaluations.begin());
}

void formula_mgr::clear()
{
    f_.clear();
    variable_to_id_.clear();
    variables_.clear();
}

auto formula_mgr::get_variable(variable_id_t id) const -> std::string
{
    assert(id < variables_.size());
    return variables_[id];
}

auto formula_mgr::change_variables_to_variable_ids(json& f) const -> bool
{
    if(!f.contains("name"))
    {
        error() << "Json (sub)formula is missing a 'name' field:\n" << f.dump(4);
        return false;
    }

    auto& name_field = f["name"];
    if(!name_field.is_string())
    {
        error() << "Json (sub)formula has 'name' field which is not a string:\n" << f.dump(4);
        return false;
    }

    if(!f.contains("value"))
    {
        error() << "Json (sub)formula is missing a 'value' field:\n" << f.dump(4);
        return false;
    }

    auto& value_field = f["value"];
    auto op = name_field.get<std::string>();
    if(op == "string") // variable
    {
        if(!value_field.is_string())
        {
            error() << "Json (sub)formula has 'value' field which is not a string:\n" << f.dump(4);
            return false;
        }
        name_field = "variable_id";

        const auto variable_str = value_field.get<std::string>();
        assert(variable_to_id_.find(variable_str) != variable_to_id_.end());
        value_field = variable_to_id_.find(variable_str)->second;
        return true;
    }

    if(value_field.is_object())
    {
        return change_variables_to_variable_ids(value_field);
    }
    if(value_field.is_array() && value_field.size() == 2)
    {
        return change_variables_to_variable_ids(value_field[0]) &&
               change_variables_to_variable_ids(value_field[1]);
    }

    error() << "Json (sub)formula has 'value' field which is neither an object, nor an array of two "
               "objects:\n"
            << f.dump(4);
    return false;
}

std::ostream& operator<<(std::ostream& out, const formula_mgr& f)
{
    out << f.f_;
    return out;
}

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
auto formula_mgr::has_satisfiable_evaluation(const formula& f, variable_evaluations_bitset_t& evaluations,
                                             variable_evaluations_bitset_t::iterator it) const -> bool
{
    if(it == evaluations.end())
    {
        auto res = f.evaluate(evaluations);
        trace() << "Evaluation with:";
        for(size_t i = 0, bound = variables_.size(); i < bound; ++i)
        {
            trace() << variables_[i] << " -> " << evaluations[i];
        }
        trace() << "\t" << (res ? "succeeded" : "failed");
        return res;
    }

    auto next = it;
    ++next;

    *it = false;
    if(has_satisfiable_evaluation(f, evaluations, next))
    {
        return true;
    }

    *it = true;
    return has_satisfiable_evaluation(f, evaluations, next);
}
