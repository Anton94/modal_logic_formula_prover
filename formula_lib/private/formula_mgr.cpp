#include "formula_mgr.h"
#include "logger.h"
#include "utils.h"

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

    auto op = name_field.get<std::string>();
    if(op == "T" || op == "F" || op == "1" || op == "0")
    {
        return true;
    }

    if(!f.contains("value"))
    {
        error() << "Json (sub)formula is missing a 'value' field:\n" << f.dump(4);
        return false;
    }
    auto& value_field = f["value"];

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

formula_mgr::formula_mgr(formula_mgr&& rhs) noexcept
    : f_(this)
{
    move(std::move(rhs));
}

formula_mgr& formula_mgr::operator=(formula_mgr&& rhs) noexcept
{
    if(this != &rhs)
    {
        move(std::move(rhs));
    }
    return *this;
}

auto formula_mgr::build(json& f) -> bool
{
    clear();

    info() << "Start building a formula";
    verbose() << f.dump(4);

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

auto formula_mgr::brute_force_evaluate(variable_to_evaluation_map_t& out_evaluations) const -> bool
{
    info() << "Running brute force evalution checking of " << f_;
    full_variables_evaluations_t evaluations(variables_.size(), false);

    bool found_satisfiable_evaluation{};
    const auto elapsed_time = time_measured_call([&]() {
        found_satisfiable_evaluation = has_satisfiable_evaluation(f_, evaluations, evaluations.begin());
    });

    info() << (found_satisfiable_evaluation ? "Success" : "Failed") << ". "
           << "Generated " << to_int(evaluations) + 1 /* + 00...0 evaluation*/ << " evaluations. "
           << "Took " << elapsed_time.count() << "ms.";

    if(found_satisfiable_evaluation)
    {
        out_evaluations.clear();
        for(size_t i = 0; i < evaluations.size(); ++i)
        {
            out_evaluations.push_back({get_variable(i), evaluations[i]});\
        }
        info() << "Evaluations: " << out_evaluations;
        return true;
    }
    return false;
}

auto formula_mgr::is_satisfiable(variable_to_evaluation_map_t& out_evaluations) -> bool
{
    info() << "Running satisfiability checking of " << f_ << "...";
    variables_evaluations_block result_evaluation_block(variables_mask_t{});

    bool is_f_satisfiable{};
    const auto elapsed_time = time_measured_call([&]() {
        is_f_satisfiable = t_.is_satisfiable(f_, result_evaluation_block);
    });

    info() << (is_f_satisfiable ? "Satisfiable" : "NOT Satisfiable") << ". "
           << "Took " << elapsed_time.count() << "ms.";
    if(is_f_satisfiable)
    {
        out_evaluations.clear();
        const auto& variables = result_evaluation_block.get_variables();
        const auto& evaluations = result_evaluation_block.get_evaluations();
        for(size_t i = 0; i < variables.size(); ++i)
        {
            if(variables[i])
            {
                out_evaluations.push_back({get_variable(i), evaluations.test(i)});
            }
        }
        info() << "Evaluations: " << out_evaluations;
        return true;
    }
    return false;
}

auto formula_mgr::does_evaluates_to_true(const variable_to_evaluation_map_t& evaluations) -> bool
{
    variables_mask_t variables_mask(variables_.size());
    for(const auto& variable_evaluation : evaluations)
    {
        const auto& name = variable_evaluation.first;
        const auto it = variable_to_id_.find(name);
        if(it == variable_to_id_.end())
        {
            error() << "The variable " << name << " is not used in the formula: " << f_;
            return false;
        }
        const auto id = it->second;

        variables_mask.set(id, true);
    }

    variables_evaluations_block evaluation_block(variables_mask);
    auto& evaluation_block_evaluations = evaluation_block.get_evaluations();
    for(const auto& variable_evaluation : evaluations)
    {
        const auto& name = variable_evaluation.first;
        const auto& evaluation = variable_evaluation.second;
        const auto it = variable_to_id_.find(name);
        assert(it != variable_to_id_.end());
        const auto id = it->second;

        evaluation_block_evaluations.set(id, evaluation);
    }

    trace() << "Trying to evaluate " << f_ << " to constant true with:";
    print(trace().get_buff(), evaluation_block);

    if(f_.does_evaluate_to_true(evaluation_block))
    {
        trace() << "    ... success";
        return true;
    }

    trace() << "    ... fail";
    return false;
}

void formula_mgr::clear()
{
    f_.clear();
    variable_to_id_.clear();
    variables_.clear();
}

auto formula_mgr::get_variables() const -> const variables_t&
{
    return variables_;
}

auto formula_mgr::get_variable(variable_id_t id) const -> std::string
{
    assert(id < variables_.size());
    return variables_[id];
}

auto formula_mgr::get_variable(const std::string& name) const -> variable_id_t
{
    auto it = variable_to_id_.find(name);
    if(it != variable_to_id_.end())
    {
        return it->second;
    }
    return variable_id_t(-1);
}

auto formula_mgr::get_internal_formula() const -> const formula*
{
    return &f_;
}

void formula_mgr::print(std::ostream& out, const variables_evaluations_block& block) const
{
    const auto& variables = block.get_variables();
    const auto& evaluations = block.get_evaluations();
    for(size_t i = 0; i < variables.size(); ++i)
    {
        if(variables[i])
        {
            out << "<" << get_variable(i) << " : " << evaluations.test(i) << "> ";
        }
    }
}

void formula_mgr::print(std::ostream& out, const variables_mask_t& variables_mask) const
{
    for(size_t i = 0; i < variables_mask.size(); ++i)
    {
        if(variables_mask[i])
        {
            out << get_variable(i) << " ";
        }
    }
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

    auto op = name_field.get<std::string>();
    if(op == "T" || op == "F" || op == "1" || op == "0")
    {
        return true;
    }

    if(!f.contains("value"))
    {
        error() << "Json (sub)formula is missing a 'value' field:\n" << f.dump(4);
        return false;
    }

    auto& value_field = f["value"];
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

void formula_mgr::move(formula_mgr&& rhs) noexcept
{
    variable_to_id_ = std::move(rhs.variable_to_id_);
    variables_ = std::move(rhs.variables_);
    f_ = std::move(rhs.f_);

    f_.change_formula_mgr(this);
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
auto formula_mgr::has_satisfiable_evaluation(const formula& f, full_variables_evaluations_t& evaluations,
                                             full_variables_evaluations_t::iterator it) const -> bool
{
    if(it == evaluations.end())
    {
        auto res = f.evaluate(evaluations);
        verbose() << "Evaluation with:";
        for(size_t i = 0, bound = variables_.size(); i < bound; ++i)
        {
            verbose() << variables_[i] << " -> " << evaluations[i];
        }
        verbose() << "\t" << (res ? "succeeded" : "failed");
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

std::ostream& operator<<(std::ostream& out, const formula_mgr& f)
{
    out << *f.get_internal_formula();
    return out;
}
