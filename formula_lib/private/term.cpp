#include "term.h"

#include <cassert>

term::term()
    : is_in_DNF_(false)
    , op_(operation_t::invalid_)
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
    assert(op_ != operation_t::invalid_);
    if(hash_ != rhs.hash_ || op_ != rhs.op_)
    {
        return false;
    }

    if (op_ == operation_t::literal_)
    {
        return variable_ == rhs.variable_;
    }

    assert(left_ && rhs.left_);
    if (op_ == operation_t::star_)
    {
        return *left_ == *rhs.left_;
    }

    assert(right_ && rhs.right_);
    return *left_ == *rhs.left_ && *right_ == *rhs.right_;
}

auto term::build(json& t) -> bool
{
    // check the json for correct information
    if(!t.contains("name"))
    {
        return false;
    }
    auto& name_field = t["name"];
    if(!name_field.is_string())
    {
        return false;
    }

    auto op = name_field.get<std::string>();
    if(op == "string")
    {
        op_ = operation_t::literal_;

        auto& value_field = t["value"];
        if (!value_field.is_string())
        {
            return false;
        }
        variable_ = value_field.get<std::string>();

        hash_ = (std::hash<std::string>()(variable_) & 0xFFFFFFFF) * 2654435761;
    }
    else if(op == "Tand")
    {
        if (!construct_binary_operation(t, operation_t::intersaction_))
        {
            return false;
        }
    }
    else if(op == "Tor")
    {
        if (!construct_binary_operation(t, operation_t::union_))
        {
            return false;
        }
    }
    else if(op == "Tstar")
    {
        op_ = operation_t::star_;

        left_ = new(std::nothrow) term();
        assert(left_);

        auto& value_field = t["value"];

        if(!value_field.is_object() || !left_->build(value_field))
        {
            return false;
        }

        hash_ = (left_->hash_ & 0xFFFFFFFF) * 2654435761;
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

auto term::construct_binary_operation(json& t, operation_t op) -> bool
{
    op_ = op;
    assert(is_binary_operaton());

    // allocate the new term childs
    left_ = new(std::nothrow) term();
    right_ = new(std::nothrow) term();
    assert(left_ && right_);

    // check the json for correct information
    auto& value_field = t["value"];
    if (!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    // recursive construction of the child terms
    if (!left_->build(value_field[0]) || !right_->build(value_field[1]))
    {
        return false;
    }

    // add child's hashes
    hash_ = ((left_->get_hash() & 0xFFFFFFFF) * 2654435761) +
        ((right_->get_hash() & 0xFFFFFFFF) * 2654435741);

    return true;
}

auto term::get_hash() const -> std::size_t
{
    return hash_;
}

std::ostream& operator<<(std::ostream& out, const term& t)
{
    switch (t.op_)
    {
        case term::operation_t::union_:
            out << "[" << *t.left_ << " - " << *t.right_<< "]";
            break;
        case term::operation_t::intersaction_:
            out << "[" << *t.left_ << " + " << *t.right_ << "]";
            break;
        case term::operation_t::star_:
            out << "[" << t.variable_ << "]*";
            break;
        case term::operation_t::literal_:
            out << t.variable_;
            break;
        case term::operation_t::invalid_:
            out << "UNDEFINED";
            break;
        default:
            assert(false && "Unrecognized.");
    }

    return out;
}

auto term::is_binary_operaton() const -> bool
{
    return op_ == operation_t::union_ || op_ == operation_t::intersaction_;
}
