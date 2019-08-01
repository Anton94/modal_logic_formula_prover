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
    auto brute_force_evaluate() const -> bool;

    // Checks if the formula is satisfiable or not
    auto is_satisfiable(human_readable_variables_evaluations_t& out_evaluations) -> bool;

    void clear();

    auto get_variables() const -> const variables_t&;

    /// Internal API
    auto get_variable(variable_id_t id) const -> std::string;
    auto get_variable(const std::string& name) const -> variable_id_t;
    auto get_internal_formula() const -> const formula*;

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
