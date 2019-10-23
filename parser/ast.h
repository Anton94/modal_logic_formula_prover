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

    union_, // union is a keyword
    intersaction,
    complement,
    variable,

    invalid,
};

class Visitor;

class Node
{
public:
    virtual void accept(Visitor& v) = 0;
    virtual ~Node() = default;
};

class NFormula : public Node
{
public:
    NFormula(formula_operation_t op, Node* left = nullptr, Node* right = nullptr);

    void accept(Visitor& v) override;

    ~NFormula() override;

    formula_operation_t op;
    Node* left;
    Node* right;
};

class NTerm : public Node
{
public:
    NTerm(term_operation_t op, Node* left = nullptr, Node* right = nullptr);

    void accept(Visitor& v) override;

    ~NTerm() override;

    term_operation_t op;
    Node* left;
    Node* right;

    std::string variable; // TODO: separate place for variable terms
};
