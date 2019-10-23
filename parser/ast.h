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

    // the caller should take care of the returned pointer(it's lifetime)
    virtual auto deep_copy() const -> Node* = 0;
};

class NFormula : public Node
{
public:
    NFormula(formula_operation_t op, Node* left = nullptr, Node* right = nullptr);

    void accept(Visitor& v) override;

    ~NFormula() override;

    auto deep_copy() const -> NFormula* override;

    formula_operation_t op;
    Node* left;
    Node* right;
};

class NTerm : public Node
{
public:
    NTerm(term_operation_t op, NTerm* left = nullptr, NTerm* right = nullptr);

    void accept(Visitor& v) override;

    ~NTerm() override;

    auto deep_copy() const -> NTerm* override;

    term_operation_t op;
    NTerm* left;
    NTerm* right;

    std::string variable; // TODO: separate place for variable terms
};
