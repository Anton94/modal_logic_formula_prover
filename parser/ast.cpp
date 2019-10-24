#include <cassert>

#include "ast.h"
#include "visitor.h"

NFormula::NFormula(formula_operation_t op, Node* left /*= nullptr*/, Node* right /*= nullptr*/)
    : op(op)
    , left(left)
    , right(right)
{
}

NFormula::~NFormula()
{
    delete left;
    delete right;
}

NFormula::NFormula(NFormula&& rhs) noexcept
{
    move(std::move(rhs));
}

NFormula& NFormula::operator=(NFormula&& rhs) noexcept
{
    if(this != &rhs)
    {
        move(std::move(rhs));
    }
    return *this;
}

auto NFormula::operator==(const NFormula& rhs) const -> bool
{
    switch(op)
    {
        case formula_operation_t::constant_true:
        case formula_operation_t::constant_false:
            return op == rhs.op;
        case formula_operation_t::less_eq:
        case formula_operation_t::measured_less_eq:
        case formula_operation_t::contact:
        {
            if(op == rhs.op)
            {
                auto l = static_cast<const NTerm*>(left);
                auto rhs_l = static_cast<const NTerm*>(rhs.left);
                auto r = static_cast<const NTerm*>(right);
                auto rhs_r = static_cast<const NTerm*>(rhs.right);

                return *l == *rhs_l && *r == *rhs_r;
            }
            return false;
        }
        case formula_operation_t::eq_zero:
        {
            if(op == rhs.op)
            {
                auto l = static_cast<const NTerm*>(left);
                auto rhs_l = static_cast<const NTerm*>(rhs.left);
                return *l == *rhs_l;
            }
            return false;
        }
        case formula_operation_t::conjunction:
        case formula_operation_t::disjunction:
        case formula_operation_t::implication:
        case formula_operation_t::equality:
        {
            if(op == rhs.op)
            {
                auto l = static_cast<const NFormula*>(left);
                auto rhs_l = static_cast<const NFormula*>(rhs.left);
                auto r = static_cast<const NFormula*>(right);
                auto rhs_r = static_cast<const NFormula*>(rhs.right);

                return *l == *rhs_l && *r == *rhs_r;
            }
            return false;
        }
        case formula_operation_t::negation:
        {
            if(op == rhs.op)
            {
                auto l = static_cast<const NFormula*>(left);
                auto rhs_l = static_cast<const NFormula*>(rhs.left);
                return *l == *rhs_l;
            }
            return false;
        }
        default:
            assert(false && "Unrecognized.");
            return false;
    }
}

auto NFormula::operator!=(const NFormula& rhs) const -> bool
{
    return !operator==(rhs);
}

void NFormula::accept(Visitor& v)
{
    v.visit(*this);
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
        case formula_operation_t::eq_zero:
        case formula_operation_t::negation:
            return new NFormula(op, left->deep_copy());
        default:
            assert(false && "Unrecognized.");
            return nullptr;
    }
}

void NFormula::move(NFormula &&rhs) noexcept
{
    delete left;
    delete right;

    op = rhs.op;
    left = rhs.left;
    right = rhs.right;

    // important, detach the @rhs's childs because otherwise rhs's destructor will destroy them
    rhs.left = rhs.right = nullptr;
}

NTerm::NTerm(term_operation_t op, NTerm* left /*= nullptr*/, NTerm* right /*= nullptr*/)
    : op(op)
    , left(left)
    , right(right)
{
}

NTerm::~NTerm()
{
    delete left;
    delete right;
}

NTerm::NTerm(NTerm&& rhs) noexcept
{
    move(std::move(rhs));
}

NTerm& NTerm::operator=(NTerm&& rhs) noexcept
{
    if(this != &rhs)
    {
        move(std::move(rhs));
    }
    return *this;
}

auto NTerm::operator==(const NTerm& rhs) const -> bool
{
    switch(op)
    {
        case term_operation_t::constant_true:
        case term_operation_t::constant_false:
            return op == rhs.op;
        case term_operation_t::variable:
            return op == rhs.op && variable == rhs.variable;
        case term_operation_t::union_:
        case term_operation_t::intersaction:
            return op == rhs.op && *left == *rhs.left;
        case term_operation_t::complement:
            return op == rhs.op && *left == *rhs.left && *right == *rhs.right;
        default:
            assert(false && "Unrecognized.");
            return false;
    }
}

auto NTerm::operator!=(const NTerm& rhs) const -> bool
{
    return !operator==(rhs);
}

void NTerm::accept(Visitor& v)
{
    v.visit(*this);
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
            return nullptr;
    }
}

void NTerm::move(NTerm &&rhs) noexcept
{
    delete left;
    delete right;

    op = rhs.op;
    left = rhs.left;
    right = rhs.right;
    variable = std::move(rhs.variable);

    // important, detach the @rhs's childs because otherwise rhs's destructor will destroy them
    rhs.left = rhs.right = nullptr;
}
