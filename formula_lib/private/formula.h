#pragma once

#include "nlohmann_json/json.hpp"
#include "types.h"

#include <ostream>

class tableau;
class term;

class formula
{
    using json = nlohmann::json;

public:
    enum class operation_type : char
    {
        conjunction,
        disjunction,
        negation,
        le,
        c,

        invalid,
    };
    using operation_t = operation_type;

    formula();
    ~formula();
    formula(const formula&) = delete;
    formula& operator=(const formula&) = delete;
    formula(formula&&) noexcept = default;
    formula& operator=(formula&&) noexcept = default;

    auto operator==(const formula& rhs) const -> bool;

    auto build(json& f) -> bool;
    auto get_hash() const -> std::size_t;

    auto get_operation_type() const -> operation_t;
    void clear();

    auto is_term_operation() const -> bool;
    auto is_atomic() const -> bool;
    auto is_formula_operation() const -> bool;

    void get_variables(variables_t& out_variables) const;
    auto evaluate(const variable_evaluations_t& variable_evaluations) const -> bool;

    friend std::ostream& operator<<(std::ostream& out, const formula& f);

private:
    friend tableau;

    auto construct_binary_term(json& f, operation_t op) -> bool;
    auto construct_binary_formula(json& f, operation_t op) -> bool;
    void free();

    operation_t op_;
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
