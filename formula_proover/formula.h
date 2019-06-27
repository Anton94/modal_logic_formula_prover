#pragma once

#include "nlohmann_json/json.hpp"
#include "term.h"
#include <ostream>

class formula
{
    using json = nlohmann::json;

public:
    formula();
    ~formula();

    formula(const formula&) = delete;
    formula& operator=(const formula&) = delete;

    formula(formula&&) noexcept = default;
    formula& operator=(formula&&) noexcept = default;

    auto operator==(const formula& rhs) const -> bool;

    auto build(json& f) -> bool;
    auto get_hash() const -> std::size_t;

    friend std::ostream& operator<<(std::ostream& out, const formula& f);

private:
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

    static auto operation_to_symbol(operation_t op) -> std::string&;
    auto is_term_operation() const -> bool;
    auto is_formula_operation() const -> bool;

    auto create_terms(json& f) -> bool;
    auto create_formulas(json& f) -> bool;

    operation_t op_;
    std::size_t hash_;
    union {
        struct
        {
            formula* left_f_;
            formula* right_f_;
        };
        struct
        {
            term* left_t_;
            term* right_t_;
        };
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
