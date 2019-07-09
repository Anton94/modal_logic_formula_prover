#pragma once

#include "nlohmann_json/json.hpp"
#include "types.h"
#include <ostream>
#include <unordered_set>

class formula_mgr;

class term
{
    using json = nlohmann::json;

public:
    term(formula_mgr* mgr);
    ~term();
    term(const term&) = delete;
    term& operator=(const term&) = delete;
    term(term&&) noexcept = default;
    term& operator=(term&&) noexcept = default;

    auto operator==(const term& rhs) const -> bool;

    auto build(json& t) -> bool;
    auto get_hash() const -> std::size_t;
    void clear();

    void get_variables(variables_set_t& out_variables) const;
    auto evaluate(const variable_evaluations_t& variable_evaluations) const -> bool;

    friend std::ostream& operator<<(std::ostream& out, const term& t);

private:
    enum class operation_type : char
    {
        union_,
        intersaction_,
        star_,
        literal_,

        invalid_,
        // TODO: encode here DNF stuff
    };
    using operation_t = operation_type;

    auto construct_binary_operation(json& t, operation_t op) -> bool;
    auto is_binary_operaton() const -> bool;
    void free();

    auto get_literal() const -> std::string;

    operation_t op_;
    formula_mgr* formula_mgr_;

    struct childs
    {
        term* left;
        term* right;
    };
    union
    {
        childs childs_;
        size_t literal_id_;
    };

    std::size_t hash_;
};

namespace std
{

template <>
struct hash<term>
{
    auto operator()(const term& t) const -> std::size_t
    {
        return t.get_hash();
    }
};
}
