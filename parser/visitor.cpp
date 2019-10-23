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

namespace
{

template<typename T>
void free_left_child(T& n)
{
    static_assert(std::is_same<T, NTerm>::value || std::is_same<T, NFormula>::value, "Only for NTerm and NFormula types!");
    delete n.left;
    n.left = nullptr;
}

template<typename T>
void free_right_child(T& n)
{
    static_assert(std::is_same<T, NTerm>::value || std::is_same<T, NFormula>::value, "Only for NTerm and NFormula types!");
    delete n.right;
    n.right = nullptr;
}

template<typename T>
void free_childs(T& n)
{
    static_assert(std::is_same<T, NTerm>::value || std::is_same<T, NFormula>::value, "Only for NTerm and NFormula types!");
    free_left_child(n);
    free_right_child(n);
}

// TODO: add move operator= and ctor to the nodes!

// moves right into left
void move(NFormula& l, NFormula& r)
{
    l.op = r.op;
    l.left = r.left;
    l.right = r.right;
    // important, detach the @r's childs because otherwise it the destructor will free them
    r.left = r.right = nullptr;
}

// moves right into left
void move(NTerm& l, NTerm& r)
{
    l.op = r.op;
    l.left = r.left;
    l.right = r.right;
    l.variable = std::move(r.variable);
    // important, detach the @r's childs because otherwise it the destructor will free them
    r.left = r.right = nullptr;
}

template<typename T>
void remove_right_child_and_move_left_child_into_parent(T& p)
{
    static_assert(std::is_same<T, NTerm>::value || std::is_same<T, NFormula>::value, "Only for NTerm and NFormula types!");
    // parent (@p): '(left OP right)' will be converted to just 'left'

    free_right_child(p);

    // move @p.left node directly into @p and delete the old left node
    T* old_left = static_cast<T*>(p.left); // T* cast is not very safe because NFormula's childs are of type Node and in theory they might not be NFormulas in future, but for now they are!

    move(p, *old_left);

    delete old_left;
}

template<typename T>
void remove_left_child_and_move_right_child_into_parent(T& p)
{
    static_assert(std::is_same<T, NTerm>::value || std::is_same<T, NFormula>::value, "Only for NTerm and NFormula types!");
    // parent (@p): '(left OP right)' will be converted to just 'right'

    free_left_child(p);

    // move @p.right node directly into @p and delete the old right node
    T* old_right = static_cast<T*>(p.right); // T* cast is not very safe because NFormula's childs are of type Node and in theory they might not be NFormulas in future, but for now they are!

    move(p, *old_right);

    delete old_right;
}

}

void VReduceTrivialAndOrNegOperations::visit(NFormula& f)
{
    switch(f.op)
    {
        case formula_operation_t::constant_true:
        case formula_operation_t::constant_false:
            break;
        case formula_operation_t::less_eq:
        case formula_operation_t::measured_less_eq:
        case formula_operation_t::implication:
        case formula_operation_t::equality:
        case formula_operation_t::contact: // TODO: reduce also C(a,0)->F ; C(0,a)->F
            f.left->accept(*this);
            f.right->accept(*this);
            break;
        case formula_operation_t::conjunction:
        {
            f.left->accept(*this);
            f.right->accept(*this);

            auto left = static_cast<NFormula*>(f.left);
            auto right = static_cast<NFormula*>(f.right);

            // 1 & 1 -> 1
            if(left->op == formula_operation_t::constant_true && right->op == formula_operation_t::constant_true)
            {
                f.op = formula_operation_t::constant_true;
                free_childs(f);
            }
            // (g & T) -> g
            else if (right->op == formula_operation_t::constant_true)
            {
                remove_right_child_and_move_left_child_into_parent(f);
            }
            // (T & g) -> g
            else if (left->op == formula_operation_t::constant_true)
            {
                remove_left_child_and_move_right_child_into_parent(f);
            }
            // (F & g) OR (g & F) -> F
            else if (left->op == formula_operation_t::constant_false || right->op == formula_operation_t::constant_false)
            {
                f.op = formula_operation_t::constant_false;
                free_childs(f);
            }

            break;
        }
        case formula_operation_t::disjunction:
        {
            f.left->accept(*this);
            f.right->accept(*this);

            auto left = static_cast<NFormula*>(f.left);
            auto right = static_cast<NFormula*>(f.right);

            // F | F -> F
            if(left->op == formula_operation_t::constant_false && right->op == formula_operation_t::constant_false)
            {
                f.op = formula_operation_t::constant_false;
                free_childs(f);
            }
            // (g | F) -> g
            else if (right->op == formula_operation_t::constant_false)
            {
                remove_right_child_and_move_left_child_into_parent(f);
            }
            // (F | g) -> g
            else if (left->op == formula_operation_t::constant_false)
            {
                remove_left_child_and_move_right_child_into_parent(f);
            }
            // (T | g) OR (g | T) -> T
            else if (left->op == formula_operation_t::constant_true || right->op == formula_operation_t::constant_true)
            {
                f.op = formula_operation_t::constant_true;
                free_childs(f);
            }

            break;
        }
        case formula_operation_t::negation:
        {
            f.left->accept(*this);

            auto left = static_cast<NFormula*>(f.left);

            // -F -> T
            if(left->op == formula_operation_t::constant_false)
            {
                f.op = formula_operation_t::constant_true;
                free_left_child(f);
            }
            // -T -> F
            else if (left->op == formula_operation_t::constant_true)
            {
                f.op = formula_operation_t::constant_false;
                free_left_child(f);
            }
            break;
        }
        default:
            assert(false && "Unrecognized.");
    }
}

void VReduceTrivialAndOrNegOperations::visit(NTerm& t)
{
    switch(t.op)
    {
        case term_operation_t::constant_true:
        case term_operation_t::constant_false:
        case term_operation_t::variable:
            break;
        case term_operation_t::union_:
        {
            t.left->accept(*this);
            t.right->accept(*this);

            // 0 + 0 -> 0
            if(t.left->op == term_operation_t::constant_false && t.right->op == term_operation_t::constant_false)
            {
                t.op = term_operation_t::constant_false;
                free_childs(t);
            }
            // (t + 0) -> t
            else if (t.right->op == term_operation_t::constant_false)
            {
                remove_right_child_and_move_left_child_into_parent(t);
            }
            // (0 + t) -> t
            else if (t.left->op == term_operation_t::constant_false)
            {
                remove_left_child_and_move_right_child_into_parent(t);
            }
            // (1 + t) OR (t + 1) -> 1
            else if (t.left->op == term_operation_t::constant_true || t.right->op == term_operation_t::constant_true)
            {
                t.op = term_operation_t::constant_true;
                free_childs(t);
            }

            break;
        }
        case term_operation_t::intersaction:
        {
            t.left->accept(*this);
            t.right->accept(*this);

            // 1 * 1 -> 1
            if(t.left->op == term_operation_t::constant_true && t.right->op == term_operation_t::constant_true)
            {
                t.op = term_operation_t::constant_true;
                free_childs(t);
            }
            // (t * 1) -> t
            else if (t.right->op == term_operation_t::constant_true)
            {
                remove_right_child_and_move_left_child_into_parent(t);
            }
            // (1 * t) -> t
            else if (t.left->op == term_operation_t::constant_true)
            {
                remove_left_child_and_move_right_child_into_parent(t);
            }
            // (0 * t) OR (t * 0) -> 0
            else if (t.left->op == term_operation_t::constant_false || t.right->op == term_operation_t::constant_false)
            {
                t.op = term_operation_t::constant_false;
                free_childs(t);
            }

            break;
        }
        case term_operation_t::complement:
        {
            t.left->accept(*this);

            // -0 -> 1
            if(t.left->op == term_operation_t::constant_false)
            {
                t.op = term_operation_t::constant_true;
                free_left_child(t);
            }
            // -1 -> 0
            else if (t.left->op == term_operation_t::constant_true)
            {
                t.op = term_operation_t::constant_false;
                free_left_child(t);
            }
            break;
        }
        default:
            assert(false && "Unrecognized.");
    }
}

void VConvertImplicationEqualityToConjDisj::visit(NFormula& f)
{
    switch(f.op)
    {
        case formula_operation_t::constant_true:
        case formula_operation_t::constant_false:
        case formula_operation_t::less_eq:
        case formula_operation_t::measured_less_eq:
        case formula_operation_t::contact:
            break;
        case formula_operation_t::conjunction:
        case formula_operation_t::disjunction:
            f.left->accept(*this);
            f.right->accept(*this);
            break;
        case formula_operation_t::negation:
            f.left->accept(*this);
            break;
        case formula_operation_t::implication:
        {
            f.left->accept(*this);
            f.right->accept(*this);

            // (f -> g)  => (~f | g)
            auto* neg_f = new NFormula(formula_operation_t::negation, f.left);
            f.left = neg_f;
            f.op = formula_operation_t::disjunction;
            // f.right stays as 'g'
            break;
        }
        case formula_operation_t::equality:
        {
            f.left->accept(*this);
            f.right->accept(*this);
            // (f <-> g) => ((f & g) | (~f & ~g))

            auto f_ = f.left; // f
            auto g_ = f.right; // g
            auto neg_f_ = new NFormula(formula_operation_t::negation, f.left->deep_copy()); // ~f
            auto neg_g_ = new NFormula(formula_operation_t::negation, f.right->deep_copy()); // ~g

            auto left_conj = new NFormula(formula_operation_t::conjunction, f_, g_); // (f & g)
            auto right_conj = new NFormula(formula_operation_t::conjunction, neg_f_, neg_g_); // (~f & ~g)

            // make the current node the disjunction one of: ((f & g) | (~f & ~g))
            f.op = formula_operation_t::disjunction;
            f.left = left_conj;
            f.right = right_conj;
            break;
        }
        default:
            assert(false && "Unrecognized.");
    }
}

void VConvertImplicationEqualityToConjDisj::visit(NTerm&)
{
}
