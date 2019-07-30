#pragma once

#include "formula_mgr.h"

#include <ostream>
#include <unordered_map>
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

    // TODO: implement the contact rule checking also when adding new =0 term

    // T(C(a,b)) has broken contact rule if a = 0 | b = 0
    auto has_broken_contact_rule_T(const formula* f) const -> bool;
    // F(C(a,b)) has broken contact rule if a != 0 & b != 0
    auto has_broken_contact_rule_F(const formula* f) const -> bool;

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
        auto operator()(const term* const& t) const -> std::size_t;
    };

    struct term_ptr_comparator
    {
        auto operator()(const term* const& lhs, const term* const& rhs) const -> bool;
    };

    using formulas_t = std::unordered_set<const formula*, formula_ptr_hasher, formula_ptr_comparator>;
    using terms_t = std::unordered_set<const term*, term_ptr_hasher, term_ptr_comparator>;

    using multiterms_t = std::unordered_multiset<const term*, term_ptr_hasher, term_ptr_comparator>;
    using multiterm_to_formula_t =
        std::unordered_multimap<const term*, const formula*, term_ptr_hasher, term_ptr_comparator>;

    // Removes only the key(*term) which has a matching address (pointer address)
    void remove_term(multiterms_t& terms, const term* t);
    // Removes the key - value pair (*t - *f) which has a matching addresses (pointer addresses)
    void remove_term_to_formula(multiterm_to_formula_t& mapping, const term* t, const formula* f);

    auto has_broken_contact_rule_new_non_zero_term(const term* t) const -> bool;

    friend std::ostream& operator<<(std::ostream& out, const formulas_t& formulas);
    friend std::ostream& operator<<(std::ostream& out, const terms_t& terms);
    friend std::ostream& operator<<(std::ostream& out, const multiterms_t& terms);
    friend std::ostream& operator<<(std::ostream& out, const multiterm_to_formula_t& mapping);

    formulas_t formulas_T_;
    formulas_t formulas_F_;
    formulas_t contacts_T_;
    formulas_t contacts_F_;
    terms_t zero_terms_T_;
    terms_t zero_terms_F_;

    // keeps the terms of the T contacts(the contacts in @contacts_T_),
    // i.e. for each T(C(a,b)) : a, b are in the collection
    multiterms_t contact_T_terms_;
    // maps the term and the F contact (the contacts in @contacts_F_) in which it belongs to,
    // i.e. for each F(C(a,b)), let C(a,b)'s pointer is 'c': a -> c and b -> c are mappings in the collection
    multiterm_to_formula_t terms_to_F_contacts_;
};
