#pragma once

#include "formula.h"
#include "term.h"
#include "types.h"
#include "variables_evaluations_block_stack.h"
#include "model.h"

#include <ostream>
#include <unordered_map>
#include <unordered_set>

class formula;
class formula_mgr;

class tableau
{
public:
    tableau() = default;
    tableau(const tableau&) = delete;
    tableau& operator=(const tableau&) = delete;
    tableau(tableau&&) = default;
    tableau& operator=(tableau&&) = default;

    // Checks if the formula is satisfiable or not
    auto is_satisfiable(const formula& f, model& out_model) -> bool;

private:
    void clear();

    // Does a step of tableau method and returns whether there was a contradiction or not
    auto satisfiable_step() -> bool;

    // T(C(a,b)) has broken contact rule if a = 0 | b = 0
    auto has_broken_contact_rule(const formula* f) const -> bool;

    auto find_in_T(const formula* f) const -> bool;
    auto find_in_F(const formula* f) const -> bool;

    void add_formula_to_T(const formula* f);
    void add_formula_to_F(const formula* f);
    void remove_formula_from_T(const formula* f);
    void remove_formula_from_F(const formula* f);

    // Removes only the key(*term) which has a matching address (pointer address)
    void remove_term(multiterms_t& terms, const term* t);

    void log_state_satisfiable() const;

    struct T_conjuction_child
    {
        T_conjuction_child(tableau& t, const formula* f);

        auto validate() const -> bool;
        void add_to_T();
        void remove_from_T();

    private:
        tableau& t_;
        const formula* f_;
        bool added_{};
    };

    struct F_disjunction_child
    {
        F_disjunction_child(tableau& t, const formula* f);

        auto validate() const -> bool;
        void add_to_F();
        void remove_from_F();

    private:
        tableau& t_;
        const formula* f_;
        bool added_{};
    };

    // TODO: explain the algorithm
    // Generates evaluations for the variables and checks if they satisfy the atomic operations.
    auto has_satisfiable_model() -> bool;

    auto get_used_variables() const -> variables_mask_t;

    auto is_contact_F_rule_satisfied() const -> bool;
    auto is_zero_term_rule_satisfied() const -> bool;

    std::ostream& print(std::ostream& out, const model::points_t& model_points_);

    const formula_mgr* mgr_;

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
    //multiterm_to_formula_t terms_to_F_contacts_;

    model model_;
};
