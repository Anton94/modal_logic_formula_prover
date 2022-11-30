#pragma once

#include "types.h"
#include "variables_evaluations_block.h"

#include <ostream>

class term;
class formula_mgr;
class NFormula;

class formula
{
public:
    formula(formula_mgr* mgr);
    ~formula();
    formula(const formula&) = delete;
    formula& operator=(const formula&) = delete;
    formula(formula&& rhs) noexcept;
    formula& operator=(formula&& rhs) noexcept;

    auto operator==(const formula& rhs) const -> bool;
    auto operator!=(const formula& rhs) const -> bool;

    auto build(const NFormula& f) -> bool;

    // TODO: remove and use the one with contact relations if it does not have negative performance impact
    auto evaluate(const variable_id_to_points_t& evals, int R, int P) const -> bool;

    // Evalates the formula but ignores all measured less or equal atomic formulas, as if they were not existing
    auto evaluate(const variable_id_to_points_t& evals, const contacts_t& contact_relations) const -> bool;

    std::pair<int, int> get_contacts_count() const;
    std::pair<int, int> get_zeroes_count() const;

    void clear();

    enum class operation_type : char
    {
        constant_true,
        constant_false,

        conjunction,
        disjunction,
        negation,

        measured_less_eq,
        eq_zero,
        c,

        invalid,
    };
    using operation_t = operation_type;

    auto get_operation_type() const -> operation_t;
    auto get_hash() const -> std::size_t;

    // The negation operation has only left child.
    // Constants has no childs.
    // UB when trying to get childs of incompatable types,
    // e.g. it is formula operation and getting a child term.
    auto get_left_child_formula() const -> const formula*;
    auto get_right_child_formula() const -> const formula*;
    auto get_left_child_term() const -> const term*;
    auto get_right_child_term() const -> const term*;

    auto get_mgr() const -> const formula_mgr*;

    auto is_term_operation() const -> bool;
    auto is_atomic() const -> bool;
    auto is_formula_operation() const -> bool;
    auto is_measured_less_eq_operation() const -> bool;
    auto is_constant() const -> bool;
    auto is_constant_true() const -> bool;
    auto is_constant_false() const -> bool;

    void change_formula_mgr(formula_mgr* new_mgr);

private:
    void move(formula&& rhs) noexcept;

    struct internal_evaluation_result
    {
        bool evaluated_value;
        bool is_used_only_less_eq_measured_as_atomic; // only the atomic <=m participates in the subformula evaluations
    };

    auto evaluate(const variable_id_to_points_t& evals, int R, int P, bool neutral_value) const -> bool;
    auto evaluate_internal(const variable_id_to_points_t& evals, const contacts_t& contact_relations) const -> internal_evaluation_result;

    auto construct_eq_zero_atomic_formula(const NFormula& f) -> bool;
    auto construct_measured_less_eq_atomic_formula(const NFormula& f) -> bool;
    auto construct_contact_atomic_formula(const NFormula& f) -> bool;
    auto construct_binary_formula(const NFormula& f, operation_t op) -> bool;
    auto construct_binary_atomic_formula(const NFormula& f, operation_t op) -> bool;
    auto construct_negation_formula(const NFormula& f) -> bool;

    void free();

    operation_t op_;
    formula_mgr* formula_mgr_;
    std::size_t hash_;

    // TODO use variant and unique_ptrs instead of raw pointers
    struct child_formulas
    {
        formula* left;
        formula* right;
    };
    struct child_terms
    {
        term* left;
        term* right;
    };

    union {
        child_formulas child_f_;
        child_terms child_t_;
    };
};

std::ostream& operator<<(std::ostream& out, const formula& f);

namespace std
{

template <>
struct hash<formula>
{
    auto operator()(const formula& f) const -> std::size_t
    {
        return f.get_hash();
    }
};
}
