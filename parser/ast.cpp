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
