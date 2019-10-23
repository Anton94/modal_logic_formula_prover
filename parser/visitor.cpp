#include "visitor.h"
#include "ast.h"


VPrinter::VPrinter(std::ostream& out)
    : out_(out)
{
}

void VPrinter::visit(NFormula& f)
{
    switch(f.op)
    {
        case formula_operation_t::constant_true:
            out_ << "T";
            break;
        case formula_operation_t::constant_false:
            out_ << "F";
            break;
        case formula_operation_t::conjunction:
        {
            out_ << "(";
            f.left->accept(*this);
            out_ << " & ";
            f.right->accept(*this);
            out_ << ")";
            break;
        }
        case formula_operation_t::disjunction:
        {
            out_ << "(";
            f.left->accept(*this);
            out_ << " | ";
            f.right->accept(*this);
            out_ << ")";
            break;
        }
        case formula_operation_t::implication:
        {
            out_ << "(";
            f.left->accept(*this);
            out_ << " -> ";
            f.right->accept(*this);
            out_ << ")";
            break;
        }
        case formula_operation_t::equality:
        {
            out_ << "(";
            f.left->accept(*this);
            out_ << " <-> ";
            f.right->accept(*this);
            out_ << ")";
            break;
        }
        case formula_operation_t::negation:
        {
            out_ << "~";
            f.left->accept(*this);
            break;
        }
        case formula_operation_t::less_eq:
        {
            out_ << "<=(";
            f.left->accept(*this);
            out_ << ", ";
            f.right->accept(*this);
            out_ << ")";
            break;
        }
        case formula_operation_t::measured_less_eq:
        {
            out_ << "<=m(";
            f.left->accept(*this);
            out_ << ", ";
            f.right->accept(*this);
            out_ << ")";
            break;
        }
        case formula_operation_t::contact:
        {
            out_ << "C(";
            f.left->accept(*this);
            out_ << ", ";
            f.right->accept(*this);
            out_ << ")";
            break;
        }
        case formula_operation_t::invalid:
            out_ << "UNDEFINED";
            break;
        default:
            assert(false && "Unrecognized.");
    }
}

void VPrinter::visit(NTerm& t)
{
    switch(t.op)
    {
        case term_operation_t::constant_true:
            out_ << "1";
            break;
        case term_operation_t::constant_false:
            out_ << "0";
            break;
        case term_operation_t::variable:
            out_ << t.variable;
            break;
        case term_operation_t::union_:
        {
            out_ << "(";
            t.left->accept(*this);
            out_ << " + ";
            t.right->accept(*this);
            out_ << ")";
            break;
        }
        case term_operation_t::intersaction:
        {
            out_ << "(";
            t.left->accept(*this);
            out_ << " * ";
            t.right->accept(*this);
            out_ << ")";
            break;
        }
        case term_operation_t::complement:
        {
            out_ << "-";
            t.left->accept(*this);
            break;
        }
        case term_operation_t::invalid:
            out_ << "UNDEFINED";
            break;
        default:
            assert(false && "Unrecognized.");
    }
}
