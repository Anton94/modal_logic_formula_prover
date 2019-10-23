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

    eq_zero,

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
    // the caller should take care of the returned pointer(it's lifetime)
    virtual auto deep_copy() const -> Node* = 0;

    virtual ~Node() = default;
};

class NFormula : public Node
{
public:
    NFormula(formula_operation_t op, Node* left = nullptr, Node* right = nullptr);
    ~NFormula() override;

    NFormula(NFormula&& rhs) noexcept;
    NFormula& operator=(NFormula&& rhs) noexcept;
    NFormula(const NFormula&) = delete;
    NFormula& operator=(const NFormula&) = delete;

    void accept(Visitor& v) override;

    auto deep_copy() const -> NFormula* override;

    formula_operation_t op;
    Node* left;
    Node* right;

private:
    void move(NFormula&& rhs) noexcept;
};

class NTerm : public Node
{
public:
    NTerm(term_operation_t op, NTerm* left = nullptr, NTerm* right = nullptr);
    ~NTerm() override;

    NTerm(NTerm&&) noexcept;
    NTerm& operator=(NTerm&&) noexcept;
    NTerm(const NTerm&) = delete;
    NTerm& operator=(const NTerm&) = delete;

    void accept(Visitor& v) override;

    auto deep_copy() const -> NTerm* override;

    term_operation_t op;
    NTerm* left;
    NTerm* right;

    std::string variable; // TODO: separate place for variable terms

private:
    void move(NTerm&& rhs) noexcept;
};
