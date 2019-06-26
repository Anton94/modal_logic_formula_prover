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
        "&", "|", "~", "<=", "C", "INVALID",
    };

    return representations[static_cast<int>(op)];
}

bool formula::is_term_operation() const
{
    return op_ == operation_t::le || op_ == operation_t::c;
}

bool formula::is_formula_operation() const
{
    return op_ == operation_t::conjunction || op_ == operation_t::disjunction || op_ == operation_t::negation;
}

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

bool formula::operator==(const formula& rhs) const
{
    assert(op_ != operation_t::invalid);
    if(hash_ != rhs.hash_ || op_ != rhs.op_)
    {
        return false;
    }

    assert(left_t_ && right_t_ && rhs.left_f_ && rhs.right_f_);
    if(is_term_operation())
    {
        return *left_t_ == *rhs.left_t_ && *right_t_ == *rhs.right_t_;
    }

    assert(is_formula_operation());
    return *left_f_ == *rhs.left_f_ && *right_f_ == *rhs.right_f_;
}

bool formula::build(json& f)
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

bool formula::create_terms(json& f)
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

bool formula::create_formulas(json& f)
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

std::size_t formula::get_hash() const
{
    return hash_;
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
