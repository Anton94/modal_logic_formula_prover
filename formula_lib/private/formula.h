#pragma once

#include "../private/types.h"
#include "nlohmann_json/json.hpp"

#include <ostream>

class term;
class formula_mgr;

class formula
{
    using json = nlohmann::json;

public:
    formula(formula_mgr* mgr);
    ~formula();
    formula(const formula&) = delete;
    formula& operator=(const formula&) = delete;
    formula(formula&& rhs) noexcept;
    formula& operator=(formula&& rhs) noexcept;

    auto operator==(const formula& rhs) const -> bool;
    auto operator!=(const formula& rhs) const -> bool;

    auto build(json& f) -> bool;
    auto evaluate(const full_variables_evaluations_t& variable_evaluations) const -> bool;
    void clear();

    enum class operation_type : char
    {
        constant_true,
        constant_false,

        conjunction,
        disjunction,
        negation,
        le,
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

    auto is_term_operation() const -> bool;
    auto is_atomic() const -> bool;
    auto is_formula_operation() const -> bool;
    auto is_constant() const -> bool;

    void change_formula_mgr(formula_mgr* new_mgr);

private:
    void move(formula&& rhs) noexcept;

    auto construct_binary_term(json& f, operation_t op) -> bool;
    auto construct_binary_formula(json& f, operation_t op) -> bool;
    void free();

    operation_t op_;
    formula_mgr* formula_mgr_;
    std::size_t hash_;

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
