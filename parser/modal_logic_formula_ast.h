#pragma once

#include <string>

enum class formula_operation_t
{
    constant_true,
    constant_false,

    conjunction,
    disjunction,
    negation,
    implication,
    equality,

    contact,
    less_eq,
    measured_less_eq,

    invalid,
};

enum class term_operation_t
{
    constant_true,
    constant_false,

    union_,
    intersaction_,
    star_,
    variable_,

    invalid_,
};

class node
{
public:
    virtual ~node() = default;
};

class formula_node : public node
{
public:
    ~formula_node() override
    {
        delete left;
        delete right;
    };

    formula_operation_t op{formula_operation_t::invalid};
    node* left{};
    node* right{};
};

class term_node : public node
{
public:
    ~term_node() override
    {
        delete left;
        delete right;
    };

    term_operation_t op{term_operation_t::invalid_};
    node* left{};
    node* right{};
};

class term_variable_node : public node
{
public:
    ~term_variable_node() override = default;

    std::string variable; // TODO: use symbol_node and flex's lookup table for the variables...
};
