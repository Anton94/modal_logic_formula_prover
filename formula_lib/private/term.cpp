#include "term.h"

#include <cassert>

term::term()
    : is_in_DNF_(false)
    , op_(operation_t::invalid)
    , left_(nullptr)
    , right_(nullptr)
    , hash_(0ul)
{
}

term::~term()
{
    delete left_;
    delete right_;
}

auto term::operator==(const term& rhs) const -> bool
{
    assert(op_ != operation_t::invalid);
    if(hash_ != rhs.hash_ || op_ != rhs.op_)
    {
        return false;
    }

    if (op_ == operation_t::literal)
    {
        return variable_ == rhs.variable_;
    }

    assert(left_ && rhs.left_);
    if (op_ == operation_t::star)
    {
        return *left_ == *rhs.left_;
    }

    assert(right_ && rhs.right_);
    return *left_ == *rhs.left_ && *right_ == *rhs.right_;
}

auto term::build(json& t) -> bool
{
    if(!t.contains("value"))
    {
        return false;
    }
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
    if(op == "Tand")
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

auto term::get_hash() const -> std::size_t
{
    return hash_;
}

std::ostream& operator<<(std::ostream& out, const term& t)
{
    out << t.term_raw.get<std::string>(); // TODO: support complex terms
    return out;
}
