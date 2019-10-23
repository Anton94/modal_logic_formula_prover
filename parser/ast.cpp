#include "ast.h"
#include "visitor.h"

NFormula::NFormula(formula_operation_t op, Node* left /*= nullptr*/, Node* right /*= nullptr*/)
    : op(op)
    , left(left)
    , right(right)
{
}

void NFormula::accept(Visitor& v)
{
    v.visit(*this);
}

NFormula::~NFormula()
{
    delete left;
    delete right;
}

auto NFormula::deep_copy() const -> NFormula*
{
    switch(op)
    {
        case formula_operation_t::constant_true:
        case formula_operation_t::constant_false:
            return new NFormula(op);
        case formula_operation_t::less_eq:
        case formula_operation_t::measured_less_eq:
        case formula_operation_t::contact:
        case formula_operation_t::conjunction:
        case formula_operation_t::disjunction:
        case formula_operation_t::implication:
        case formula_operation_t::equality:
            return new NFormula(op, left->deep_copy(), right->deep_copy());
        case formula_operation_t::negation:
            return new NFormula(op, left->deep_copy());
        default:
            assert(false && "Unrecognized.");
    }
}

NTerm::NTerm(term_operation_t op, NTerm* left /*= nullptr*/, NTerm* right /*= nullptr*/)
    : op(op)
    , left(left)
    , right(right)
{
}

void NTerm::accept(Visitor& v)
{
    v.visit(*this);
}

NTerm::~NTerm()
{
    delete left;
    delete right;
}

auto NTerm::deep_copy() const -> NTerm*
{
    switch(op)
    {
        case term_operation_t::constant_true:
        case term_operation_t::constant_false:
            return new NTerm(op);
        case term_operation_t::variable:
        {
            auto copy = new NTerm(op);
            copy->variable = variable;
            return copy;
        }
        case term_operation_t::union_:
        case term_operation_t::intersaction:
            return new NTerm(op, left->deep_copy(), right->deep_copy());
        case term_operation_t::complement:
            return new NTerm(op, left->deep_copy());
        default:
            assert(false && "Unrecognized.");
    }
}
