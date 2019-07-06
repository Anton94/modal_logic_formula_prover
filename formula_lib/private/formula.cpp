#include "formula.h"

#include <cassert>

formula::formula()
    : op_(operation_t::invalid)
    , hash_(0ul)
    , child_f_({nullptr, nullptr})
{
}

formula::~formula()
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
    else
    {
        delete child_f_.left;
        delete child_f_.right;
    }
}

auto formula::operator==(const formula& rhs) const -> bool
{
    assert(op_ != operation_t::invalid);
    if(hash_ != rhs.hash_ || op_ != rhs.op_)
    {
        return false;
    }

    assert(child_f_.left && rhs.child_f_.left);
    if (op_ == operation_t::negation)
    {
        return *child_f_.left == *rhs.child_f_.left;
    }

    assert(child_f_.right && rhs.child_f_.right);
    if(is_term_operation())
    {
        return *child_t_.left == *rhs.child_t_.left && *child_t_.right == *rhs.child_t_.right;
    }

    assert(is_formula_operation());
    return *child_f_.left == *rhs.child_f_.left && *child_f_.right == *rhs.child_f_.right;
}

auto formula::build(json& f) -> bool
{
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
    if (op == "conjunction")
    {
        if (!construct_binary_formula(f, operation_t::conjunction))
        {
            return false;
        }
    }
    else if (op == "disjunction")
    {
        if (!construct_binary_formula(f, operation_t::disjunction))
        {
            return false;
        }
    }
    else if(op == "less")
    {
        if (!construct_binary_term(f, operation_t::le))
        {
            return false;
        }
    }
    else if(op == "contact")
    {
        if(!construct_binary_term(f, operation_t::c))
        {
            return false;
        }
    }
    else if(op == "negation")
    {
        op_ = operation_t::negation;

        child_f_.left = new(std::nothrow) formula();
        assert(child_f_.left);

        auto& value_field = f["value"];

        if(!value_field.is_object() || !child_f_.left->build(value_field))
        {
            return false;
        }

        hash_ = (child_f_.left->hash_ & 0xFFFFFFFF) * 2654435761;
    }
    else
    {
        assert(false && "Unrecognized operation :(");
        return false;
    }

    // add also the operation to the hash
    const auto op_code = static_cast<unsigned>(op_) + 1;
    hash_ += (op_code & 0xFFFFFFFF) * 2654435723;

    return true;
}

auto formula::is_term_operation() const -> bool
{
	return op_ == operation_t::le || op_ == operation_t::c;
}

auto formula::is_atomic() const -> bool
{
    return is_term_operation();
}

auto formula::is_formula_operation() const -> bool
{
	return op_ == operation_t::conjunction || op_ == operation_t::disjunction || op_ == operation_t::negation;
}

auto formula::construct_binary_term(json& f, operation_t op) -> bool
{
    op_ = op;
    assert(is_term_operation());

    child_t_.left = new(std::nothrow) term();
    child_t_.right = new(std::nothrow) term();
    assert(child_t_.left && child_t_.right);

    // check the json for correct information
    auto& value_field = f["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    // recursive construction of the child terms
    if (!child_t_.left->build(value_field[0]) || !child_t_.right->build(value_field[1]))
    {
        return false;
    }

    // add child's hashes
    hash_ = ((child_f_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
        ((child_f_.right->get_hash() & 0xFFFFFFFF) * 2654435741);
    return true;
}

auto formula::construct_binary_formula(json& f, operation_t op) -> bool
{
    op_ = op;
    assert(op_ == operation_t::conjunction || op_ == operation_t::disjunction);

    child_f_.left = new(std::nothrow) formula();
    child_f_.right = new(std::nothrow) formula();
    assert(child_f_.left && child_f_.right);

    // check the json for correct information
    auto& value_field = f["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    // recursive construction of the child terms
    if (!child_f_.left->build(value_field[0]) || !child_f_.right->build(value_field[1]))
    {
        return false;
    }

    // add child's hashes
    hash_ = ((child_f_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
        ((child_f_.right->get_hash() & 0xFFFFFFFF) * 2654435741);
    return true;
}

auto formula::get_hash() const -> std::size_t
{
    return hash_;
}

auto formula::get_operation_type() const -> operation_t
{
    return op_;
}

std::ostream& operator<<(std::ostream& out, const formula& f)
{
    switch (f.op_)
    {
    case formula::operation_t::conjunction:
        out << "[" << *f.child_f_.left << " & " << *f.child_f_.right << "]";
        break;
    case formula::operation_t::disjunction:
        out << "[" << *f.child_f_.left << " | " << *f.child_f_.right << "]";
        break;
    case formula::operation_t::negation:
        out << "~" << *f.child_f_.left;
        break;
    case formula::operation_t::le:
        out << "<=[" << *f.child_t_.left << ", " << *f.child_t_.right << "]";
        break;
    case formula::operation_t::c:
        out << "C[" << *f.child_t_.left << ", " << *f.child_t_.right << "]";
        break;
    case formula::operation_t::invalid:
        out << "UNDEFINED";
        break;
    default:
        assert(false && "Unrecognized.");
    }

    return out;
}
