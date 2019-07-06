#pragma once

#include "nlohmann_json/json.hpp"
#include "../private/term.h"

#include <ostream>

class tableau;

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

    auto is_term_operation() const -> bool;
    auto is_atomic() const -> bool;
    auto is_formula_operation() const -> bool;

    friend std::ostream& operator<<(std::ostream& out, const formula& f);

private:
    friend tableau;

    static auto operation_to_symbol(operation_t op) -> std::string&;

    auto create_terms(json& f) -> bool;
    auto create_formulas(json& f) -> bool;

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