#include "formula.h"

#include <cassert>

formula::formula()
    : op_(operation_t::invalid)
    , hash_(0ul)
    , left_f_(nullptr)
    , right_f_(nullptr)
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
        delete left_t_;
        delete right_t_;
    }
    else
    {
        delete left_f_;
        delete right_f_;
    }
}

auto formula::operator==(const formula& rhs) const -> bool
{
    assert(op_ != operation_t::invalid);
    if(hash_ != rhs.hash_ || op_ != rhs.op_)
    {
        return false;
    }

    assert(left_f_ && rhs.left_f_);
    if (op_ == operation_t::negation)
    {
        return *left_f_ == *rhs.left_f_;
    }

    assert(right_f_ && rhs.right_f_);
    if(is_term_operation())
    {
        return *left_t_ == *rhs.left_t_ && *right_t_ == *rhs.right_t_;
    }

    assert(is_formula_operation());
    return *left_f_ == *rhs.left_f_ && *right_f_ == *rhs.right_f_;
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
    if(op == "le")
    {
        op_ = operation_t::le;

        if(!create_terms(f))
        {
            return false;
        }

        hash_ = ((left_t_->get_hash() & 0xFFFFFFFF) * 2654435761) +
                ((right_t_->get_hash() & 0xFFFFFFFF) * 2654435741);
    }
    else if(op == "C")
    {
        op_ = operation_t::c;

        if(!create_terms(f))
        {
            return false;
        }

        hash_ = ((left_t_->get_hash() & 0xFFFFFFFF) * 2654435761) +
                ((right_t_->get_hash() & 0xFFFFFFFF) * 2654435741);
    }
    else if(op == "and")
    {
        op_ = operation_t::conjunction;

        if(!create_formulas(f))
        {
            return false;
        }

        hash_ = ((left_f_->get_hash() & 0xFFFFFFFF) * 2654435761) +
                ((right_f_->get_hash() & 0xFFFFFFFF) * 2654435741);
    }
    else if(op == "or")
    {
        op_ = operation_t::disjunction;

        if(!create_formulas(f))
        {
            return false;
        }

        hash_ = ((left_f_->get_hash() & 0xFFFFFFFF) * 2654435761) +
                ((right_f_->get_hash() & 0xFFFFFFFF) * 2654435741);
    }
    else if(op == "neg")
    {
        op_ = operation_t::negation;

        left_f_ = new(std::nothrow) formula();
        assert(left_f_);

        auto& value_field = f["value"];

        if(!value_field.is_object() || !left_f_->build(value_field))
        {
            return false;
        }

        hash_ = (left_f_->hash_ & 0xFFFFFFFF) * 2654435761;
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
    left_t_ = new(std::nothrow) term();
    right_t_ = new(std::nothrow) term();
    assert(left_t_ && right_t_);

    auto& value_field = f["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    return left_t_->build(value_field[0]) && right_t_->build(value_field[1]);
}

auto formula::create_formulas(json& f) -> bool
{
    left_f_ = new(std::nothrow) formula();
    right_f_ = new(std::nothrow) formula();
    assert(left_f_ && right_f_);

    auto& value_field = f["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    return left_f_->build(value_field[0]) && right_f_->build(value_field[1]);
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
        out << "[" << formula::operation_to_symbol(f.op_) << "[" << *f.left_t_ << ", " << *f.right_t_ << "]]";
    }
    else if(f.op_ == formula::operation_t::negation)
    {
        out << "[" << formula::operation_to_symbol(f.op_) << *f.left_f_ << "]";
    }
    else
    {
        out << "[" << *f.left_f_ << " " << formula::operation_to_symbol(f.op_) << " " << *f.right_f_ << "]";
    }

    return out;
}
