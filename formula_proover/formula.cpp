#include "formula.h"

#include <cassert>

std::string& formula::operation_to_symbol(operation_t op)
{
	assert(static_cast<char>(operation_t::conjunction) == 0 &&
		static_cast<char>(operation_t::disjunction) == 1 &&
		static_cast<char>(operation_t::negation) == 2 &&
		static_cast<char>(operation_t::le) == 3 &&
		static_cast<char>(operation_t::c) == 4 &&
		static_cast<char>(operation_t::invalid) == 5);
	static std::string representations[] = {
		"&",
		"|",
		"~",
		"<=",
		"c",
		"INVALID",
	};

	return representations[static_cast<int>(op)];
}

bool formula::is_term_operation() const
{
	return op_ == operation_t::le || op_ == operation_t::c;
}

formula::formula()
	: op_(operation_t::invalid)
	, left_f_(nullptr)
	, right_f_(nullptr)
{
}

formula::~formula()
{
	if (op_ == operation_t::invalid)
	{
		return;
	}

	if (is_term_operation())
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

bool formula::build(json& f)
{
	if (!f.contains("name"))
	{
		return false;
	}

	auto& name_field = f["name"];
	if (!name_field.is_string())
	{
		return false;
	}

	auto op = name_field.get<std::string>();
	if (op == "le")
	{
		op_ = operation_t::le;

		left_t_ = new (std::nothrow) term();
		right_t_ = new (std::nothrow) term();
		assert(left_t_ && right_t_);

		auto& value_field = f["value"];
		if (!value_field.is_array() || value_field.size() != 2)
		{
			return false;
		}

		left_t_->build(value_field[0]);
		right_t_->build(value_field[1]);
	}
	// else if (op == "c")
	else if (op == "and")
	{
		op_ = operation_t::conjunction;

		left_f_ = new (std::nothrow) formula();
		right_f_ = new (std::nothrow) formula();
		assert(left_f_ && right_f_);

		auto& value_field = f["value"];
		if (!value_field.is_array() || value_field.size() != 2)
		{
			return false;
		}

		left_f_->build(value_field[0]);
		right_f_->build(value_field[1]);
	}
	else if (op == "or")
	{
		op_ = operation_t::disjunction;

		left_f_ = new (std::nothrow) formula();
		right_f_ = new (std::nothrow) formula();
		assert(left_f_ && right_f_);

		auto& value_field = f["value"];
		if (!value_field.is_array() || value_field.size() != 2)
		{
			return false;
		}

		left_f_->build(value_field[0]);
		right_f_->build(value_field[1]);
	}
	else if (op == "neg")
	{
		op_ = operation_t::negation;

		left_f_ = new (std::nothrow) formula();
		assert(left_f_);

		auto& value_field = f["value"];
		if (!value_field.is_object())
		{
			return false;
		}

		left_f_->build(value_field);
	}
	else
	{
		assert(false && "Unrecognized operation :(");
		return false;
	}

	return true;
}

std::ostream& operator<<(std::ostream& out, const formula& f)
{
	if (f.op_ == formula::operation_t::invalid)
	{
		out << "UNDEFINED";
	}
	else if (f.is_term_operation())
	{
		out << "(" << *f.left_t_ << " " << formula::operation_to_symbol(f.op_) << " " << *f.right_t_ << ")";
	}
	else if (f.op_ == formula::operation_t::negation)
	{
		out << "(" << formula::operation_to_symbol(f.op_) << *f.left_f_ << ")";
	}
	else
	{
		out << "(" << *f.left_f_ << " " << formula::operation_to_symbol(f.op_) << " " << *f.right_f_ << ")";
	}

	return out;
}
