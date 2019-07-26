#include "formula.h"
#include "logger.h"
#include "term.h"

#include <cassert>

formula::formula(formula_mgr* mgr)
    : op_(operation_t::invalid)
    , formula_mgr_(mgr)
    , hash_(0ul)
    , child_f_({nullptr, nullptr})
{
    assert(formula_mgr_);
}

formula::formula(formula&& rhs) noexcept
{
    move(std::move(rhs));
}

formula& formula::operator=(formula&& rhs) noexcept
{
    if(this != &rhs)
    {
        move(std::move(rhs));
    }
    return *this;
}

formula::~formula()
{
    free();
}

auto formula::operator==(const formula& rhs) const -> bool
{
    assert(op_ != operation_t::invalid);
    if(hash_ != rhs.hash_ || op_ != rhs.op_)
    {
        return false;
    }

    if(is_constant()) // note that the operations in the two objects are the same
    {
        return true;
    }

    assert(child_f_.left && rhs.child_f_.left);
    if(op_ == operation_t::negation)
    {
        return *child_f_.left == *rhs.child_f_.left;
    }

    assert(child_f_.right && rhs.child_f_.right);
    if(op_ == operation_t::eq_zero)
    {
        return *child_t_.left == *rhs.child_t_.left;
    }
    if(op_ == operation_t::c)
    {
        // C is commutative, i.e. C(a,b) == C(b,a)
        // C(a, b) == C(c, d) => (a == c && b == d) v (a == d && b == c)
        return (*child_t_.left == *rhs.child_t_.left && *child_t_.right == *rhs.child_t_.right) ||
               (*child_t_.left == *rhs.child_t_.right && *child_t_.right == *rhs.child_t_.left);
    }

    assert(is_formula_operation());
    return *child_f_.left == *rhs.child_f_.left && *child_f_.right == *rhs.child_f_.right;
}

auto formula::operator!=(const formula& rhs) const -> bool
{
    return !operator==(rhs);
}

auto formula::build(json& f) -> bool
{
    clear();

    // check the json for correct information
    if(!f.contains("name"))
    {
        return false;
    }
    auto& name_field = f["name"];
    if(!name_field.is_string())
    {
        return false;
    }

    auto op = name_field.get<std::string>();
    if(op == "constant_T")
    {
        op_ = operation_t::constant_true;
    }
    else if(op == "constant_F")
    {
        op_ = operation_t::constant_false;
    }
    else if(op == "conjunction")
    {
        if(!construct_binary_formula(f, operation_t::conjunction))
        {
            return false;
        }
    }
    else if(op == "disjunction")
    {
        if(!construct_binary_formula(f, operation_t::disjunction))
        {
            return false;
        }
    }
    else if(op == "equal0")
    {
        if(!construct_eq_zero_atomic_formula(f))
        {
            return false;
        }
    }
    else if(op == "contact")
    {
        if(!construct_contact_atomic_formula(f))
        {
            return false;
        }
    }
    else if(op == "negation")
    {
        if(!construct_negation_formula(f))
        {
            return false;
        }
    }
    else
    {
        assert(false && "Unrecognized operation :(");
        return false;
    }

    // add also the operation to the hash
    const auto op_code = static_cast<unsigned>(op_) + 1;
    hash_ += (op_code & 0xFFFFFFFF) * 2654435723;

    trace() << *this << " <" << hash_ << ">";
    return true;
}

auto formula::evaluate(const full_variables_evaluations_t& variable_evaluations) const -> bool
{
    switch(op_)
    {
        case formula::operation_t::constant_true:
            return true;
        case formula::operation_t::constant_false:
            return false;
        case formula::operation_t::conjunction:
            assert(child_f_.left && child_f_.right);
            return child_f_.left->evaluate(variable_evaluations) &&
                   child_f_.right->evaluate(variable_evaluations);
        case formula::operation_t::disjunction:
            assert(child_f_.left && child_f_.right);
            return child_f_.left->evaluate(variable_evaluations) ||
                   child_f_.right->evaluate(variable_evaluations);
        case formula::operation_t::negation:
            assert(child_f_.left);
            return !child_f_.left->evaluate(variable_evaluations);
        case formula::operation_t::eq_zero:
            assert(child_t_.left);
            return !child_t_.left->evaluate(variable_evaluations);
        case formula::operation_t::c:
            assert(child_t_.left && child_t_.right);
            // C(a, b) is satisfied if a != 0 and b != 0
            return child_t_.left->evaluate(variable_evaluations) &&
                   child_t_.right->evaluate(variable_evaluations);
        default:
            assert(false && "Unrecognized.");
            return false;
    }
}

void formula::clear()
{
    free();
    op_ = operation_t::invalid;
    hash_ = 0;
    child_f_ = {nullptr, nullptr};
}

auto formula::get_operation_type() const -> operation_t
{
    return op_;
}

auto formula::get_hash() const -> std::size_t
{
    return hash_;
}

auto formula::get_left_child_formula() const -> const formula*
{
    return child_f_.left;
}

auto formula::get_right_child_formula() const -> const formula*
{
    return child_f_.right;
}

auto formula::get_left_child_term() const -> const term*
{
    return child_t_.left;
}

auto formula::get_right_child_term() const -> const term*
{
    return child_t_.right;
}

auto formula::is_term_operation() const -> bool
{
    return op_ == operation_t::eq_zero || op_ == operation_t::c;
}

auto formula::is_atomic() const -> bool
{
    return is_term_operation();
}

auto formula::is_formula_operation() const -> bool
{
    return op_ == operation_t::conjunction || op_ == operation_t::disjunction || op_ == operation_t::negation;
}

auto formula::is_constant() const -> bool
{
    return op_ == operation_t::constant_true || op_ == operation_t::constant_false;
}

void formula::change_formula_mgr(formula_mgr* new_mgr)
{
    assert(new_mgr);
    formula_mgr_ = new_mgr;

    if(op_ == operation_t::eq_zero)
    {
        child_t_.left->change_formula_mgr(new_mgr);
    }
    else if(op_ == operation_t::c)
    {
        child_t_.left->change_formula_mgr(new_mgr);
        child_t_.right->change_formula_mgr(new_mgr);
    }
    else if(op_ == operation_t::negation)
    {
        child_f_.left->change_formula_mgr(new_mgr);
    }
    else if(is_formula_operation())
    {
        child_f_.left->change_formula_mgr(new_mgr);
        child_f_.right->change_formula_mgr(new_mgr);
    }
}

std::ostream& operator<<(std::ostream& out, const formula& f)
{
    switch(f.get_operation_type())
    {
        case formula::operation_t::constant_true:
            out << "T";
            break;
        case formula::operation_t::constant_false:
            out << "F";
            break;
        case formula::operation_t::conjunction:
            out << "(" << *f.get_left_child_formula() << " & " << *f.get_right_child_formula() << ")";
            break;
        case formula::operation_t::disjunction:
            out << "(" << *f.get_left_child_formula() << " | " << *f.get_right_child_formula() << ")";
            break;
        case formula::operation_t::negation:
            out << "~" << *f.get_left_child_formula();
            break;
        case formula::operation_t::eq_zero:
            out << "(" << *f.get_left_child_term() << ") = 0";
            break;
        case formula::operation_t::c:
            out << "C(" << *f.get_left_child_term() << ", " << *f.get_right_child_term() << ")";
            break;
        case formula::operation_t::invalid:
            out << "UNDEFINED";
            break;
        default:
            assert(false && "Unrecognized.");
    }

    return out;
}

void formula::move(formula&& rhs) noexcept
{
    op_ = rhs.op_;
    formula_mgr_ = rhs.formula_mgr_;
    hash_ = rhs.hash_;

    if(is_term_operation())
    {
        child_t_ = rhs.child_t_;
    }
    else if(is_formula_operation())
    {
        child_f_ = rhs.child_f_;
    }

    // invalidate the rhs in order to not touch/deletes the moved resources, e.g. the childs
    rhs.op_ = operation_t::invalid;
}

auto formula::construct_eq_zero_atomic_formula(json& f) -> bool
{
    op_ = operation_t::eq_zero;

    child_t_.left = new(std::nothrow) term(formula_mgr_);
    assert(child_t_.left);

    auto& value_field = f["value"];
    if(!value_field.is_object() || !child_t_.left->build(value_field))
    {
        return false;
    }

    // add child's hashes
    hash_ = ((child_t_.left->get_hash() & 0xFFFFFFFF) * 2654435761);

    return true;
}

auto formula::construct_contact_atomic_formula(json& f) -> bool
{
    op_ = operation_t::c;

    child_t_.left = new(std::nothrow) term(formula_mgr_);
    child_t_.right = new(std::nothrow) term(formula_mgr_);
    assert(child_t_.left && child_t_.right);

    // check the json for correct information
    auto& value_field = f["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    // recursive construction of the child terms
    if(!child_t_.left->build(value_field[0]) || !child_t_.right->build(value_field[1]))
    {
        return false;
    }

    // add child's hashes and multiply be the same factor because the operation is commutative
    hash_ = ((child_t_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
            ((child_t_.right->get_hash() & 0xFFFFFFFF) * 2654435761);
    return true;
}

auto formula::construct_negation_formula(json& f) -> bool
{
    op_ = operation_t::negation;

    child_f_.left = new(std::nothrow) formula(formula_mgr_);
    assert(child_f_.left);

    auto& value_field = f["value"];

    if(!value_field.is_object() || !child_f_.left->build(value_field))
    {
        return false;
    }

    hash_ = (child_f_.left->get_hash() & 0xFFFFFFFF) * 2654435761;
    return true;
}

auto formula::construct_binary_formula(json& f, operation_t op) -> bool
{
    op_ = op;
    assert(op_ == operation_t::conjunction || op_ == operation_t::disjunction);

    child_f_.left = new(std::nothrow) formula(formula_mgr_);
    child_f_.right = new(std::nothrow) formula(formula_mgr_);
    assert(child_f_.left && child_f_.right);

    // check the json for correct information
    auto& value_field = f["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    // recursive construction of the child terms
    if(!child_f_.left->build(value_field[0]) || !child_f_.right->build(value_field[1]))
    {
        return false;
    }

    // add child's hashes
    hash_ = ((child_f_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
            ((child_f_.right->get_hash() & 0xFFFFFFFF) * 2654435741);
    return true;
}

void formula::free()
{
    if(op_ == operation_t::invalid)
    {
        return;
    }

    if(is_term_operation())
    {
        delete child_t_.left;
        delete child_t_.right;
    }
    else if(is_formula_operation())
    {
        delete child_f_.left;
        delete child_f_.right;
    }
}
