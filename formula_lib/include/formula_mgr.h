#pragma once

// TODO: pimpl ideom. will hide the private includes, etc.

#include "../private/formula.h"
#include "../private/tableau.h"
#include "../private/types.h"

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class formula_mgr
{
    using json = nlohmann::json;

public:
    formula_mgr();
    formula_mgr(const formula_mgr&) = delete;
    formula_mgr& operator=(const formula_mgr&) = delete;
    formula_mgr(formula_mgr&& rhs) noexcept;
    formula_mgr& operator=(formula_mgr&& rhs) noexcept;

    auto build(json& f) -> bool;

    // Generates all sequences of 0/1 to evaluate the variables and checks if the formula is satisfied
    // O(2^n) where n is the number of different variables
    auto brute_force_evaluate(variable_to_evaluation_map_t& out_evaluations) const -> bool;

    // Checks if the formula is satisfiable or not and if so it fills a subset of variables and their evaluations to the out_evaluations collection
    auto is_satisfiable(variable_to_evaluation_map_t& out_evaluations) -> bool;

    // Checks if the formula evaluates to the constant true or not with the given subset of variable evaluations
    auto does_evaluates_to_true(const variable_to_evaluation_map_t& evaluations) -> bool;

    void clear();

    auto get_variables() const -> const variables_t&;

    /// Internal API
    auto get_variable(variable_id_t id) const -> std::string;
    auto get_variable(const std::string& name) const -> variable_id_t;
    auto get_internal_formula() const -> const formula*;

    void print(std::ostream& out, const variables_evaluations_block& block) const;
    void print(std::ostream& out, const variables_mask_t& variables_mask) const;

private:
    void move(formula_mgr&& rhs) noexcept;

    auto has_satisfiable_evaluation(const formula& f, full_variables_evaluations_t& evaluations,
                                    full_variables_evaluations_t::iterator it) const -> bool;
    auto change_variables_to_variable_ids(json& f) const -> bool;

    using variable_to_id_map_t = std::unordered_map<std::string, variable_id_t>;
    variable_to_id_map_t variable_to_id_;

    variables_t variables_;
    formula f_;
    tableau t_;
};

std::ostream& operator<<(std::ostream& out, const formula_mgr& formulas);
