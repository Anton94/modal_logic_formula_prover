#pragma once

#include "formula.h"
#include "term.h"
#include "types.h"
#include "variables_evaluations_block_stack.h"

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
    auto is_satisfiable(const formula& f) -> bool;

private:
    void clear();

    // Does a step of tableau method and returns whether there was a contradiction or not
    auto satisfiable_step() -> bool;

    // T(C(a,b)) has broken contact rule if a = 0 | b = 0
    auto has_broken_contact_rule_T(const formula* f) const -> bool;
    // F(C(a,b)) has broken contact rule if a != 0 & b != 0
    auto has_broken_contact_rule_F(const formula* f) const -> bool;

    auto has_broken_contact_rule_new_non_zero_term(const term* t) const -> bool;

    auto find_in_T(const formula* f) const -> bool;
    auto find_in_F(const formula* f) const -> bool;

    void add_formula_to_T(const formula* f);
    void add_formula_to_F(const formula* f);
    void remove_formula_from_T(const formula* f);
    void remove_formula_from_F(const formula* f);

    // Removes only the key(*term) which has a matching address (pointer address)
    void remove_term(multiterms_t& terms, const term* t);
    // Removes the key - value pair (*t - *f) which has a matching addresses (pointer addresses)
    void remove_term_to_formula(multiterm_to_formula_t& mapping, const term* t, const formula* f);

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
    auto path_has_satisfiable_variable_evaluation() -> bool;

    void clear_satisfiable_variable_evaluation();
    void cache_used_variables_in_contact_F_and_zero_terms();
    auto construct_contact_term_evaluation(const term* t) -> bool;
    auto construct_non_zero_term_evaluation(const term* t, variables_evaluations_block& out_evaluation) const -> bool;

    // Generates new evaluation until @t evalautes to constant true with it
    auto generate_next_positive_evaluation(const term* t, variables_evaluations_block& evaluation) const -> bool;

    // Each contact term & evaluation (t, ev) should satisfy the following rules for each atomic formula of the following types:
    //  - ~C(d1, d2) implies that 'd1' evaluated with @ev should be zero OR 'd2' evaluated with @ev should be zero
    //  - a = 0 implies that 'a' evaluated with @ev should be zero
    auto is_contact_F_rule_satisfied(const variables_evaluations_block& evaluation) const -> bool;
    auto is_zero_term_rule_satisfied(const variables_evaluations_block& evaluation) const -> bool;

    auto has_satisfiable_contact_evaluation_for_non_zero_term(const term* t) const -> bool;

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
    multiterm_to_formula_t terms_to_F_contacts_;

    using term_to_evaluation_map_t =
        std::unordered_map<const term*, variables_evaluations_block, term_ptr_hasher, term_ptr_comparator>;
    std::ostream& print(std::ostream& out, const term_to_evaluation_map_t& term_to_evalation);

    term_to_evaluation_map_t contact_T_terms_to_evaluation_;
    term_to_evaluation_map_t zero_terms_F_to_evaluation_;

    variables_mask_t variables_in_contact_F_and_zero_terms_;
};
