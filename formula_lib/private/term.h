#pragma once

#include "../private/types.h"
#include "nlohmann_json/json.hpp"
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

    // void get_variables(variables_set_t& out_variables) const;
    auto evaluate(const full_variables_evaluations_t& variable_evaluations) const -> bool;

    void to_negative_form();

    friend std::ostream& operator<<(std::ostream& out, const term& t);

private:
    enum class operation_type : char
    {
        constant_true,
        constant_false,

        union_,
        intersaction_,
        star_,
        variable_,

        invalid_,
        // TODO: encode here DNF stuff
    };
    using operation_t = operation_type;

    term(operation_t operation, term* left = nullptr, term* right = nullptr);

    auto construct_binary_operation(json& t, operation_t op) -> bool;
    auto is_binary_operaton() const -> bool;
    auto is_constant() const -> bool;
    void free();

    auto get_variable() const -> std::string;

    operation_t op_;
    formula_mgr* formula_mgr_;

    struct childs
    {
        term* left;
        term* right;
    };
    union {
        childs childs_;
        size_t variable_id_;
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
