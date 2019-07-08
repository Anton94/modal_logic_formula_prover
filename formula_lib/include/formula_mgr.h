#pragma once

#include "../private/formula.h" // TODO: pimpl ideom
#include "types.h"

#include <ostream>
#include <unordered_set>

class tableau;

class formula_mgr
{
    using json = nlohmann::json;

public:
    formula_mgr() = default;
    formula_mgr(const formula_mgr&) = delete;
    formula_mgr& operator=(const formula_mgr&) = delete;
    formula_mgr(formula_mgr&&) = default;
    formula_mgr& operator=(formula_mgr&&) noexcept = default;

    auto build(json& f) -> bool;
    void get_variables(variables_t& out_variables) const;

    // Generates all sequences of 0/1 for to evaluate the variables and checks if the formula is satisfied
    // O(2^n) where n is the number of variables
    auto brute_force_evaluate() const -> bool;

    void clear();

private:
    friend class tableau;
    friend std::ostream& operator<<(std::ostream& out, const formula_mgr& formulas);

    formula f_;
};
