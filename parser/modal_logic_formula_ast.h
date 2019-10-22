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

class Node
{
public:
    virtual ~Node() = default;
};

class NFormula : public Node
{
public:
    NFormula(formula_operation_t op, Node* left = nullptr, Node* right = nullptr)
        : op(op)
        , left(left)
        , right(right)
    {
    }

    ~NFormula() override
    {
        delete left;
        delete right;
    }

    formula_operation_t op;
    Node* left;
    Node* right;
};

class NTerm : public Node
{
public:
    NTerm(term_operation_t op, Node* left = nullptr, Node* right = nullptr)
        : op(op)
        , left(left)
        , right(right)
    {
    }

    ~NTerm() override
    {
        delete left;
        delete right;
    }

    term_operation_t op;
    Node* left;
    Node* right;

    std::string variable; // TODO: separate place for variable terms
};
