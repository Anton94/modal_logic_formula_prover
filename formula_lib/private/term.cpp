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

term::term(term&& rhs) noexcept
{
    move(std::move(rhs));
}

term& term::operator=(term&& rhs) noexcept
{
    if (this != &rhs)
    {
        move(std::move(rhs));
    }
    return *this;
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

    if (is_constant()) // note that the operations in the two objects are the same
    {
        return true;
    }

    if(op_ == operation_t::variable_)
    {
        return variable_id_ == rhs.variable_id_;
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
    if (op == "constant_1")
    {
        construct_constant(operation_t::constant_true);
    }
    else if (op == "constant_0")
    {
        construct_constant(operation_t::constant_false);
    }
    else if(op == "variable_id")
    {
        op_ = operation_t::variable_;

        auto& value_field = t["value"];
        if(!value_field.is_number_unsigned())
        {
            return false;
        }
        variable_id_ = value_field.get<size_t>();
        const auto all_variables_count = formula_mgr_->get_variables().size();

        assert(variable_id_ < all_variables_count);
        variables_.resize(all_variables_count);
        variables_.set(variable_id_);

        hash_ = (variable_id_ & 0xFFFFFFFF) * 2654435761;
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

        variables_ = childs_.left->variables_;
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

auto term::evaluate(const full_variables_evaluations_t& variable_evaluations) const -> bool
{
    switch (op_)
    {
    case term::operation_t::constant_true:
        return true;
    case term::operation_t::constant_false:
        return false;
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
    case term::operation_t::variable_:
        assert(variable_id_ < variable_evaluations.size());
        return variable_evaluations[variable_id_]; // returns the evaluation for the variable
    default:
        assert(false && "Unrecognized.");
        return false;
    }
}

void term::clear()
{
    free();
    op_ = operation_t::invalid_;
    childs_ = { nullptr, nullptr };
    hash_ = 0;
}

auto term::get_operation_type() const-> operation_t
{
    return op_;
}

auto term::get_hash() const -> std::size_t
{
    return hash_;
}

auto term::get_variable() const -> std::string
{
    assert(op_ == operation_t::variable_);
    return formula_mgr_->get_variable(variable_id_);
}

auto term::get_variables() const -> const variables_mask_t&
{
    return variables_;
}

auto term::get_left_child() const -> const term*
{
    return childs_.left;
}

auto term::get_right_child() const -> const term*
{
    return childs_.right;
}

auto term::is_binary_operaton() const -> bool
{
    return op_ == operation_t::union_ || op_ == operation_t::intersaction_;
}

auto term::is_constant() const -> bool
{
    return op_ == operation_t::constant_true || op_ == operation_t::constant_false;
}

void term::change_formula_mgr(formula_mgr* new_mgr)
{
    assert(new_mgr);
    formula_mgr_ = new_mgr;

    switch (get_operation_type())
    {
    case operation_t::union_:
    case operation_t::intersaction_:
        childs_.left->change_formula_mgr(new_mgr);
        childs_.right->change_formula_mgr(new_mgr);
        break;
    case operation_t::star_:
        childs_.left->change_formula_mgr(new_mgr);
        break;
    default:
        break;
    }
}

std::ostream& operator<<(std::ostream& out, const term& t)
{
    switch(t.get_operation_type())
    {
        case term::operation_t::constant_true:
            out << "1";
            break;
        case term::operation_t::constant_false:
            out << "0";
            break;
        case term::operation_t::union_:
            out << "[" << *t.get_left_child() << " + " << *t.get_right_child() << "]";
            break;
        case term::operation_t::intersaction_:
            out << "[" << *t.get_left_child() << " * " << *t.get_right_child() << "]";
            break;
        case term::operation_t::star_:
            out << "-" << *t.get_left_child();
            break;
        case term::operation_t::variable_:
            out << t.get_variable();
            break;
        case term::operation_t::invalid_:
            out << "UNDEFINED";
            break;
        default:
            assert(false && "Unrecognized.");
    }

    return out;
}

void term::move(term&& rhs) noexcept
{
    op_ = rhs.op_;
    formula_mgr_ = rhs.formula_mgr_;
    variables_ = std::move(rhs.variables_);
    
    if (is_binary_operaton() || op_ == operation_t::star_)
    {
        childs_ = rhs.childs_;
    }
    else if (op_ == operation_t::variable_)
    {
        variable_id_ = rhs.variable_id_;
    }

    hash_ = rhs.hash_;

    // invalidate the rhs in order to not touch/deletes the moved resources, e.g. the childs
    rhs.op_ = operation_t::invalid_;
}

void term::construct_constant(operation_t op)
{
    op_ = op;
    assert(is_constant());

    variables_.resize(formula_mgr_->get_variables().size());
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
    if (!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    // recursive construction of the child terms
    if (!childs_.left->build(value_field[0]) || !childs_.right->build(value_field[1]))
    {
        return false;
    }

    // add child's hashes
    hash_ = ((childs_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
        ((childs_.right->get_hash() & 0xFFFFFFFF) * 2654435741);

    variables_ = childs_.left->variables_ | childs_.right->variables_;

    return true;
}

void term::free()
{
    if(op_ != operation_t::variable_ && !is_constant())
    {
        delete childs_.left;
        delete childs_.right;
    }
}
