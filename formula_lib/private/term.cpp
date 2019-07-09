#include "term.h"
#include "formula_mgr.h"
#include "logger.h"

#include <cassert>

term::term(formula_mgr* mgr)
    : op_(operation_t::invalid_)
    , formula_mgr_(mgr)
    , childs_{nullptr, nullptr}
    , hash_(0ul)
{
    assert(formula_mgr_);
}

term::~term()
{
    free();
}

auto term::operator==(const term& rhs) const -> bool
{
    assert(op_ != operation_t::invalid_);
    if(hash_ != rhs.hash_ || op_ != rhs.op_)
    {
        return false;
    }

    if(op_ == operation_t::literal_)
    {
        return literal_id_ == rhs.literal_id_;
    }

    assert(childs_.left && rhs.childs_.left);
    if(op_ == operation_t::star_)
    {
        return *childs_.left == *rhs.childs_.left;
    }

    assert(childs_.right && rhs.childs_.right);
    return *childs_.left == *rhs.childs_.left && *childs_.right == *rhs.childs_.right;
}

auto term::build(json& t) -> bool
{
    clear();

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
    if(op == "literal_id")
    {
        op_ = operation_t::literal_;

        auto& value_field = t["value"];
        if(!value_field.is_number_unsigned())
        {
            return false;
        }
        literal_id_ = value_field.get<size_t>();

        hash_ = (literal_id_ & 0xFFFFFFFF) * 2654435761;
    }
    else if(op == "Tand")
    {
        if(!construct_binary_operation(t, operation_t::intersaction_))
        {
            return false;
        }
    }
    else if(op == "Tor")
    {
        if(!construct_binary_operation(t, operation_t::union_))
        {
            return false;
        }
    }
    else if(op == "Tstar")
    {
        op_ = operation_t::star_;

        childs_.left = new(std::nothrow) term(formula_mgr_);
        assert(childs_.left);

        auto& value_field = t["value"];

        if(!value_field.is_object() || !childs_.left->build(value_field))
        {
            return false;
        }

        hash_ = (childs_.left->get_hash() & 0xFFFFFFFF) * 2654435761;
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

auto term::get_hash() const -> std::size_t
{
    return hash_;
}

auto term::is_binary_operaton() const -> bool
{
    return op_ == operation_t::union_ || op_ == operation_t::intersaction_;
}

auto term::construct_binary_operation(json& t, operation_t op) -> bool
{
    op_ = op;
    assert(is_binary_operaton());

    // allocate the new term childs
    childs_.left = new(std::nothrow) term(formula_mgr_);
    childs_.right = new(std::nothrow) term(formula_mgr_);
    assert(childs_.left && childs_.right);

    // check the json for correct information
    auto& value_field = t["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    // recursive construction of the child terms
    if(!childs_.left->build(value_field[0]) || !childs_.right->build(value_field[1]))
    {
        return false;
    }

    // add child's hashes
    hash_ = ((childs_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
            ((childs_.right->get_hash() & 0xFFFFFFFF) * 2654435741);

    return true;
}

void term::clear()
{
    free();
    op_ = operation_t::invalid_;
    childs_ = {nullptr, nullptr};
    hash_ = 0;
}

void term::get_variables(variables_set_t& out_variables) const
{
    if(is_binary_operaton())
    {
        assert(childs_.left && childs_.right);
        childs_.left->get_variables(out_variables);
        childs_.right->get_variables(out_variables);
    }
    else if(op_ == operation_t::star_)
    {
        assert(childs_.left);
        childs_.left->get_variables(out_variables);
    }
    else
    {
        assert(op_ == operation_t::literal_);
        out_variables.insert(formula_mgr_->get_literal(literal_id_));
    }
}

auto term::evaluate(const variable_evaluations_t& variable_evaluations) const -> bool
{
    switch(op_)
    {
        case term::operation_t::union_:
            assert(childs_.left && childs_.right);
            return childs_.left->evaluate(variable_evaluations) ||
                   childs_.right->evaluate(variable_evaluations);
        case term::operation_t::intersaction_:
            assert(childs_.left && childs_.right);
            return childs_.left->evaluate(variable_evaluations) &&
                   childs_.right->evaluate(variable_evaluations);
        case term::operation_t::star_:
            assert(childs_.left);
            return !childs_.left->evaluate(variable_evaluations);
        case term::operation_t::literal_:
            assert(variable_evaluations.find(get_literal()) != variable_evaluations.end());
            return variable_evaluations.find(get_literal())->second; // returns the evaluation for the variable
        default:
            assert(false && "Unrecognized.");
            return false;
    }
}

std::ostream& operator<<(std::ostream& out, const term& t)
{
    switch(t.op_)
    {
        case term::operation_t::union_:
            out << "[" << *t.childs_.left << " - " << *t.childs_.right << "]";
            break;
        case term::operation_t::intersaction_:
            out << "[" << *t.childs_.left << " + " << *t.childs_.right << "]";
            break;
        case term::operation_t::star_:
            out << "[" << *t.childs_.left << "]*";
            break;
        case term::operation_t::literal_:
            out << t.get_literal();
            break;
        case term::operation_t::invalid_:
            out << "UNDEFINED";
            break;
        default:
            assert(false && "Unrecognized.");
    }

    return out;
}

void term::free()
{
    if (op_ != operation_t::literal_)
    {
        delete childs_.left;
        delete childs_.right;
    }
}

auto term::get_literal() const -> std::string
{
    assert(op_ == operation_t::literal_);
    return formula_mgr_->get_literal(literal_id_);
}
