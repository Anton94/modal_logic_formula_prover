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
    if(op == "less")
    {
        op_ = operation_t::le;

        if(!create_terms(f))
        {
            return false;
        }

        hash_ = ((child_t_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
                ((child_t_.right->get_hash() & 0xFFFFFFFF) * 2654435741);
    }
    else if(op == "contact")
    {
        op_ = operation_t::c;

        if(!create_terms(f))
        {
            return false;
        }

        hash_ = ((child_t_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
                ((child_t_.right->get_hash() & 0xFFFFFFFF) * 2654435741);
    }
    else if(op == "conjunction")
    {
        op_ = operation_t::conjunction;

        if(!create_formulas(f))
        {
            return false;
        }

        hash_ = ((child_f_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
                ((child_f_.right->get_hash() & 0xFFFFFFFF) * 2654435741);
    }
    else if(op == "disjunction")
    {
        op_ = operation_t::disjunction;

        if(!create_formulas(f))
        {
            return false;
        }

        hash_ = ((child_f_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
                ((child_f_.right->get_hash() & 0xFFFFFFFF) * 2654435741);
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

    const auto op_code = (static_cast<unsigned>(op_) + 7) * 31;
    hash_ += (op_code & 0xFFFFFFFF) * 2654435723;

    return true;
}

auto formula::operation_to_symbol(operation_t op) -> std::string&
{
	assert(static_cast<char>(operation_t::conjunction) == 0 &&
		static_cast<char>(operation_t::disjunction) == 1 &&
		static_cast<char>(operation_t::negation) == 2 &&
		static_cast<char>(operation_t::le) == 3 &&
		static_cast<char>(operation_t::c) == 4 &&
		static_cast<char>(operation_t::invalid) == 5);
	static std::string representations[] = {
		"&", "|", "~", "<=", "C", "INVALID",
	};

	return representations[static_cast<int>(op)];
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

auto formula::create_terms(json& f) -> bool
{
    child_t_.left = new(std::nothrow) term();
    child_t_.right = new(std::nothrow) term();
    assert(child_t_.left && child_t_.right);

    auto& value_field = f["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    return child_t_.left->build(value_field[0]) && child_t_.right->build(value_field[1]);
}

auto formula::create_formulas(json& f) -> bool
{
    child_f_.left = new(std::nothrow) formula();
    child_f_.right = new(std::nothrow) formula();
    assert(child_f_.left && child_f_.right);

    auto& value_field = f["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    return child_f_.left->build(value_field[0]) && child_f_.right->build(value_field[1]);
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
    if(f.op_ == formula::operation_t::invalid)
    {
        out << "UNDEFINED";
    }
    else if(f.is_term_operation())
    {
        out << "[" << formula::operation_to_symbol(f.op_) << "[" << *f.child_t_.left << ", " << *f.child_t_.right << "]]";
    }
    else if(f.op_ == formula::operation_t::negation)
    {
        out << "[" << formula::operation_to_symbol(f.op_) << *f.child_f_.left << "]";
    }
    else
    {
        out << "[" << *f.child_f_.left << " " << formula::operation_to_symbol(f.op_) << " " << *f.child_f_.right << "]";
    }

    return out;
}
