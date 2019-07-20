#pragma once

// TODO: pimpl ideom. The impls will have in their public API the needed internals and no extra friends...

#include "../private/formula.h"
#include "../private/types.h"

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class tableau;
class term;

class formula_mgr
{
    using json = nlohmann::json;

public:
    formula_mgr();
    formula_mgr(const formula_mgr&) = delete;
    formula_mgr& operator=(const formula_mgr&) = delete;
    formula_mgr(formula_mgr&&) = default;
    formula_mgr& operator=(formula_mgr&&) noexcept = default;

    auto build(json& f) -> bool;

    using variables_t = std::vector<std::string>;
    auto get_variables() const -> const variables_t&;

    // Generates all sequences of 0/1 to evaluate the variables and checks if the formula is satisfied
    // O(2^n) where n is the number of different variables
    auto brute_force_evaluate() const -> bool;

    void clear();

private:
    friend class tableau;
    friend class formula;
    friend class term;
    friend std::ostream& operator<<(std::ostream& out, const formula_mgr& formulas);

    auto has_satisfiable_evaluation(const formula& f, full_variables_evaluations_t& evaluations,
                                    full_variables_evaluations_t::iterator it) const -> bool;

    auto get_variable(variable_id_t id) const -> std::string;
    auto change_variables_to_variable_ids(json& f) const -> bool;

    using variable_to_id_map_t = std::unordered_map<std::string, variable_id_t>;
    variable_to_id_map_t variable_to_id_;

    variables_t variables_;

    formula f_;
};

std::ostream& operator<<(std::ostream& out, const formula_mgr::variables_t& variables);
