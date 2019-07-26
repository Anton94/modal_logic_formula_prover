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
    term(term&& rhs) noexcept;
    term& operator=(term&& rhs) noexcept;

    auto operator==(const term& rhs) const -> bool;
    auto operator!=(const term& rhs) const -> bool;

    auto build(json& t) -> bool;
    auto evaluate(const full_variables_evaluations_t& variable_evaluations) const -> bool;
    void clear();

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

    auto get_operation_type() const -> operation_t;
    auto get_hash() const -> std::size_t;
    auto get_variable() const -> std::string;
    auto get_variables() const -> const variables_mask_t&;

    // The star operation has only left child
    // Constants and variable operations has no childs.
    // UB when trying to get childs of incompatable types,
    // e.g. it is star operation and taking right child
    auto get_left_child() const -> const term*;
    auto get_right_child() const -> const term*;

    void change_formula_mgr(formula_mgr* new_mgr);

private:
    void move(term&& rhs) noexcept;

    void construct_constant(operation_t op);
    auto construct_binary_operation(json& t, operation_t op) -> bool;

    auto is_binary_operaton() const -> bool;
    auto is_constant() const -> bool;
    void free();

    operation_t op_;
    formula_mgr* formula_mgr_;

    variables_mask_t variables_; // TODO: alternative is to calculate the used variables each time when we
                                 // need them. would it be better?

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

std::ostream& operator<<(std::ostream& out, const term& t);

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
