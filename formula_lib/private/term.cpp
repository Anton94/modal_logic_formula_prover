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
    if(this != &rhs)
    {
        free();
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

    if(is_constant()) // note that the operations in the two objects are the same
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

auto term::operator!=(const term& rhs) const -> bool
{
    return !operator==(rhs);
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
    if(op == "1")
    {
        op_ = operation_t::constant_true;
    }
    else if(op == "0")
    {
        op_ = operation_t::constant_false;
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
    }
    else
    {
        assert(false && "Unrecognized operation :(");
        return false;
    }

    construct_hash();
    construct_variables();

    trace() << *this << " <" << hash_ << ">";
    return true;
}

auto term::evaluate(const full_variables_evaluations_t& variable_evaluations) const -> bool
{
    switch(op_)
    {
        case operation_t::constant_true:
            return true;
        case operation_t::constant_false:
            return false;
        case operation_t::union_:
            assert(childs_.left && childs_.right);
            return childs_.left->evaluate(variable_evaluations) ||
                   childs_.right->evaluate(variable_evaluations);
        case operation_t::intersaction_:
            assert(childs_.left && childs_.right);
            return childs_.left->evaluate(variable_evaluations) &&
                   childs_.right->evaluate(variable_evaluations);
        case operation_t::star_:
            assert(childs_.left);
            return !childs_.left->evaluate(variable_evaluations);
        case operation_t::variable_:
            assert(variable_id_ < variable_evaluations.size());
            return variable_evaluations[variable_id_]; // returns the evaluation for the variable
        default:
            assert(false && "Unrecognized.");
            return false;
    }
}

auto term::evaluate(const variables_evaluations_block& evaluation_block) const -> evaluation_result
{
    using res_type = evaluation_result::result_type;
    switch(op_)
    {
        case operation_t::constant_true:
            return {res_type::constant_true, nullptr};
        case operation_t::constant_false:
            return {res_type::constant_false, nullptr};
        case operation_t::union_:
        {
            auto res_left = childs_.left->evaluate(evaluation_block);
            if(res_left.is_constant_true())
            {
                return {res_type::constant_true, nullptr};
            }

            auto res_right = childs_.right->evaluate(evaluation_block);
            if(res_right.is_constant_true())
            {
                res_left.free();
                return {res_type::constant_true, nullptr};
            }

            if(res_left.is_constant_false() && res_right.is_constant_false())
            {
                return {res_type::constant_false, nullptr};
            }

            if(res_left.is_constant_false())
            {
                assert(res_right.type == res_type::term);
                return {res_type::term, res_right.release()};
            }
            if(res_right.is_constant_false())
            {
                assert(res_left.type == res_type::term);
                return {res_type::term, res_left.release()};
            }

            assert(res_left.type == res_type::term && res_right.type == res_type::term);

            auto t = create_internal_node(op_, res_left.release(), res_right.release());
            return {res_type::term, t};
        }
        case operation_t::intersaction_:
        {
            auto res_left = childs_.left->evaluate(evaluation_block);
            if(res_left.is_constant_false())
            {
                return {res_type::constant_false, nullptr};
            }

            auto res_right = childs_.right->evaluate(evaluation_block);
            if(res_right.is_constant_false())
            {
                res_left.free();
                return {res_type::constant_false, nullptr};
            }

            if(res_left.is_constant_true() && res_right.is_constant_true())
            {
                return {res_type::constant_true, nullptr};
            }

            if(res_left.is_constant_true())
            {
                assert(res_right.type == res_type::term);
                return {res_type::term, res_right.release()};
            }
            if(res_right.is_constant_true())
            {
                assert(res_left.type == res_type::term);
                return {res_type::term, res_left.release()};
            }

            assert(res_left.type == res_type::term && res_right.type == res_type::term);

            auto t = create_internal_node(op_, res_left.release(), res_right.release());
            return {res_type::term, t};
        }
        case operation_t::star_:
        {
            auto res_child = childs_.left->evaluate(evaluation_block);
            if(res_child.is_constant())
            {
                const auto negated_constant = res_child.type == res_type::constant_true
                                                  ? res_type::constant_false
                                                  : res_type::constant_true;
                return {negated_constant, nullptr};
            }
            assert(res_child.type == res_type::term);

            auto t = create_internal_node(op_, res_child.release());
            return {res_type::term, t};
        }
        case operation_t::variable_:
        {
            const auto& block_variables = evaluation_block.get_variables();
            assert(variable_id_ < block_variables.size());
            if(block_variables[variable_id_]) // if the variable_id is participating in that block evaluation
            {
                const auto& block_evaluations = evaluation_block.get_evaluations();
                const auto type =
                    block_evaluations[variable_id_] ? res_type::constant_true : res_type::constant_false;
                return {type, nullptr};
            }

            auto t = create_variable_node(variable_id_);
            return {res_type::term, t};
        }
        default:
            assert(false && "Unrecognized.");
            return {res_type::none, nullptr};
    }
}

void term::clear()
{
    free();
    op_ = operation_t::invalid_;
    childs_ = {nullptr, nullptr};
    hash_ = 0;
}

auto term::get_operation_type() const -> operation_t
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

    switch(get_operation_type())
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

    if(is_binary_operaton() || op_ == operation_t::star_)
    {
        childs_ = rhs.childs_;
    }
    else if(op_ == operation_t::variable_)
    {
        variable_id_ = rhs.variable_id_;
    }

    hash_ = rhs.hash_;

    // invalidate the rhs in order to not touch/deletes the moved resources, e.g. the childs
    rhs.op_ = operation_t::invalid_;
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

    return true;
}

void term::construct_hash()
{
    switch(op_)
    {
        case operation_t::constant_true:
        case operation_t::constant_false:
            break;
        case operation_t::union_:
        case operation_t::intersaction_:
            hash_ = ((childs_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
                    ((childs_.right->get_hash() & 0xFFFFFFFF) * 2654435741);
            break;
        case operation_t::star_:
            hash_ = (childs_.left->get_hash() & 0xFFFFFFFF) * 2654435761;
            break;
        case operation_t::variable_:
            hash_ = (variable_id_ & 0xFFFFFFFF) * 2654435761;
            break;
        default:
            assert(false && "Unrecognized.");
    }

    // add also the operation to the hash
    const auto op_code = static_cast<unsigned>(op_) + 1;
    hash_ += (op_code & 0xFFFFFFFF) * 2654435723;
}

void term::construct_variables()
{
    switch(op_)
    {
        case operation_t::constant_true:
        case operation_t::constant_false:
            variables_.resize(formula_mgr_->get_variables().size());
            break;
        case operation_t::union_:
        case operation_t::intersaction_:
            variables_ = childs_.left->variables_ | childs_.right->variables_;
            break;
        case operation_t::star_:
            variables_ = childs_.left->variables_;
            break;
        case operation_t::variable_:
        {
            const auto all_variables_count = formula_mgr_->get_variables().size();
            assert(variable_id_ < all_variables_count);
            variables_.resize(all_variables_count);
            variables_.set(variable_id_);
            break;
        }
        default:
            assert(false && "Unrecognized.");
    }
}

auto term::create_internal_node(operation_t op, term* left, term* right) const -> term*
{
    assert(op == operation_t::star_ || op == operation_t::union_ || op == operation_t::intersaction_);

    auto t = new(std::nothrow) term(formula_mgr_);
    assert(t);

    t->op_ = op;
    t->childs_.left = left;
    t->childs_.right = right;
    t->construct_hash();
    t->construct_variables();

    return t;
}

auto term::create_variable_node(size_t variable_id) const -> term*
{
    auto t = new(std::nothrow) term(formula_mgr_);
    assert(t);

    t->op_ = operation_t::variable_;
    t->variable_id_ = variable_id;
    t->construct_hash();
    t->construct_variables();

    return t;
}

void term::free()
{
    if(op_ != operation_t::variable_ && !is_constant())
    {
        delete childs_.left;
        delete childs_.right;
    }
}

term::evaluation_result::evaluation_result(result_type res_type, term* t)
    : type(res_type)
    , t_(t)
{
}

term::evaluation_result::evaluation_result(evaluation_result&& rhs) noexcept
{
    move(std::move(rhs));
}

term::evaluation_result& term::evaluation_result::operator=(evaluation_result&& rhs) noexcept
{
    if(this != &rhs)
    {
        free();
        move(std::move(rhs));
    }
    return *this;
}

term::evaluation_result::~evaluation_result()
{
    if(t_)
    {
        const auto msg = "There is a term evaluation result with not freed term!!!";
        error() << msg;
        assert(false && msg);
        delete t_;
    }
}

term* term::evaluation_result::release()
{
    type = result_type::none;
    auto t = t_;
    t_ = nullptr;
    return t;
}

void term::evaluation_result::free()
{
    delete t_;
    t_ = nullptr;
    type = result_type::none;
}

void term::evaluation_result::move(evaluation_result&& rhs) noexcept
{
    type = rhs.type;
    t_ = rhs.t_;
    rhs.type = result_type::none;
    rhs.t_ = nullptr;
}

auto term::evaluation_result::is_constant() const -> bool
{
    return type == result_type::constant_true || type == result_type::constant_false;
}

auto term::evaluation_result::is_constant_true() const -> bool
{
    return type == result_type::constant_true;
}

auto term::evaluation_result::is_constant_false() const -> bool
{
    return type == result_type::constant_false;
}
