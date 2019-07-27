#pragma once

#include "formula_mgr.h"

#include <ostream>
#include <unordered_set>

class formula;
class term;

class tableau
{
public:
    tableau() = default;
    tableau(const tableau&) = delete;
    tableau& operator=(const tableau&) = delete;
    tableau(tableau&&) = default;
    tableau& operator=(tableau&&) noexcept = default;

    // Checks if the formula is satisfiable or not
    auto is_satisfiable(const formula_mgr& f) -> bool;

private:
    void clear();

    // Does a step of tableau method and returns whether there was a contradiction or not
    auto step() -> bool;

    auto find_in_T(const formula* f) const -> bool;
    auto find_in_F(const formula* f) const -> bool;

    void add_formula_to_T(const formula* f);
    void add_formula_to_F(const formula* f);
    void remove_formula_from_T(const formula* f);
    void remove_formula_from_F(const formula* f);

    void log_state() const;

    struct formula_ptr_hasher
    {
        auto operator()(const formula* const& f) const -> std::size_t;
    };

    struct formula_ptr_comparator
    {
        auto operator()(const formula* const& lhs, const formula* const& rhs) const -> bool;
    };

    struct term_ptr_hasher
    {
        auto operator()(const term* const& t) const->std::size_t;
    };

    struct term_ptr_comparator
    {
        auto operator()(const term* const& lhs, const term* const& rhs) const -> bool;
    };

    using formulas_t = std::unordered_set<const formula*, formula_ptr_hasher, formula_ptr_comparator>;
    using terms_t = std::unordered_set<const term*, term_ptr_hasher, term_ptr_comparator>;

    formulas_t formulas_T_;
    formulas_t formulas_F_;
    formulas_t contacts_T_;
    formulas_t contacts_F_;
    terms_t zero_terms_T_;
    terms_t zero_terms_F_;

    friend std::ostream& operator<<(std::ostream& out, const tableau::formulas_t& formulas);
    friend std::ostream& operator<<(std::ostream& out, const tableau::terms_t& formulas);
};
