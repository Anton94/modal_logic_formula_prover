#include "formula.h"
#include "logger.h"
#include "term.h"

#include <cassert>

formula::formula(formula_mgr* mgr)
    : op_(operation_t::invalid)
    , formula_mgr_(mgr)
    , hash_(0ul)
    , child_f_({nullptr, nullptr})
{
    assert(formula_mgr_);
}

formula::formula(formula&& rhs) noexcept
{
    move(std::move(rhs));
}

formula& formula::operator=(formula&& rhs) noexcept
{
    if(this != &rhs)
    {
        free();
        move(std::move(rhs));
    }
    return *this;
}

formula::~formula()
{
    free();
}

auto formula::operator==(const formula& rhs) const -> bool
{
    assert(op_ != operation_t::invalid);
    if(hash_ != rhs.hash_ || op_ != rhs.op_)
    {
        return false;
    }

    if(is_constant()) // note that the operations in the two objects are the same
    {
        return true;
    }

    if(op_ == operation_t::negation)
    {
        assert(child_f_.left && rhs.child_f_.left);
        return *child_f_.left == *rhs.child_f_.left;
    }

    assert(child_t_.left && rhs.child_t_.left);
    if(op_ == operation_t::eq_zero)
    {
        return *child_t_.left == *rhs.child_t_.left;
    }
    assert(child_t_.right && rhs.child_t_.right);
    if(op_ == operation_t::c)
    {
        // C is commutative, i.e. C(a,b) == C(b,a)
        // C(a, b) == C(c, d) => (a == c && b == d) v (a == d && b == c)
        return (*child_t_.left == *rhs.child_t_.left && *child_t_.right == *rhs.child_t_.right) ||
               (*child_t_.left == *rhs.child_t_.right && *child_t_.right == *rhs.child_t_.left);
    }

    assert(is_formula_operation());
    return *child_f_.left == *rhs.child_f_.left && *child_f_.right == *rhs.child_f_.right;
}

auto formula::operator!=(const formula& rhs) const -> bool
{
    return !operator==(rhs);
}

auto formula::build(json& f) -> bool
{
    clear();

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
    if(op == "T")
    {
        op_ = operation_t::constant_true;
    }
    else if(op == "F")
    {
        op_ = operation_t::constant_false;
    }
    else if(op == "conjunction")
    {
        if(!construct_binary_formula(f, operation_t::conjunction))
        {
            return false;
        }
    }
    else if(op == "disjunction")
    {
        if(!construct_binary_formula(f, operation_t::disjunction))
        {
            return false;
        }
    }
    else if(op == "equal0")
    {
        if(!construct_eq_zero_atomic_formula(f))
        {
            return false;
        }
    }
    else if(op == "contact")
    {
        if(!construct_contact_atomic_formula(f))
        {
            return false;
        }
    }
    else if(op == "negation")
    {
        if(!construct_negation_formula(f))
        {
            return false;
        }
    }
    else
    {
        assert(false && "Unrecognized operation :(");
        return false;
    }

    // add also the operation to the hash
    const auto op_code = static_cast<unsigned>(op_) + 1;
    hash_ += (op_code & 0xFFFFFFFF) * 2654435723;

    verbose() << "[Building] " << *this << " <" << hash_ << ">";
    return true;
}

auto formula::evaluate(const full_variables_evaluations_t& variable_evaluations) const -> bool
{
    switch(op_)
    {
        case formula::operation_t::constant_true:
            return true;
        case formula::operation_t::constant_false:
            return false;
        case formula::operation_t::conjunction:
            assert(child_f_.left && child_f_.right);
            return child_f_.left->evaluate(variable_evaluations) &&
                   child_f_.right->evaluate(variable_evaluations);
        case formula::operation_t::disjunction:
            assert(child_f_.left && child_f_.right);
            return child_f_.left->evaluate(variable_evaluations) ||
                   child_f_.right->evaluate(variable_evaluations);
        case formula::operation_t::negation:
            assert(child_f_.left);
            return !child_f_.left->evaluate(variable_evaluations);
        case formula::operation_t::eq_zero:
            assert(child_t_.left);
            return !child_t_.left->evaluate(variable_evaluations);
        case formula::operation_t::c:
            assert(child_t_.left && child_t_.right);
            // C(a, b) is satisfied if a != 0 and b != 0
            return child_t_.left->evaluate(variable_evaluations) &&
                   child_t_.right->evaluate(variable_evaluations);
        default:
            assert(false && "Unrecognized.");
            return false;
    }
}

auto formula::evaluate(relations_t& relations, std::vector<variable_evaluation_set>& variables) const -> bool
{
    switch(op_)
    {
        case formula::operation_t::constant_true:
            return true;
        case formula::operation_t::constant_false:
            return false;
        case formula::operation_t::conjunction:
            assert(child_f_.left && child_f_.right);
            return child_f_.left->evaluate(relations, variables) &&
                   child_f_.right->evaluate(relations, variables);
        case formula::operation_t::disjunction:
            assert(child_f_.left && child_f_.right);
            return child_f_.left->evaluate(relations, variables) ||
                   child_f_.right->evaluate(relations, variables);
        case formula::operation_t::negation:
            assert(child_f_.left);
            return !child_f_.left->evaluate(relations, variables);
        case formula::operation_t::eq_zero:
            assert(child_t_.left);
            return child_t_.left->evaluate(relations, variables).empty();
        case formula::operation_t::c:
        {
            assert(child_t_.left && child_t_.right);
            auto left = child_t_.left->evaluate(relations, variables);
            auto right = child_t_.right->evaluate(relations, variables);

            for(auto i = left.begin(); i != left.end(); i++)
            {
                for(auto j = right.begin(); j != right.end(); j++)
                {
                    if(*i == *j || // reflexivity of the contact
                       relations.find(std::pair<int, int>(*i, *j)) != relations.end() ||
                       relations.find(std::pair<int, int>(*j, *i)) != relations.end()) // symetry
                    {
                        return true;
                    }
                }
            }
            return false;
        }
        default:
            assert(false && "Unrecognized.");
            return false;
    }
}

auto formula::evaluate(const std::vector<variables_evaluations_t>& evals, int R, int P) const -> bool
{
    switch(op_)
    {
        case formula::operation_t::constant_true:
            return true;
        case formula::operation_t::constant_false:
            return false;
        case formula::operation_t::conjunction:
            assert(child_f_.left && child_f_.right);
            return child_f_.left->evaluate(evals, R, P) && child_f_.right->evaluate(evals, R, P);
        case formula::operation_t::disjunction:
            assert(child_f_.left && child_f_.right);
            return child_f_.left->evaluate(evals, R, P) || child_f_.right->evaluate(evals, R, P);
        case formula::operation_t::negation:
            assert(child_f_.left);
            return !child_f_.left->evaluate(evals, R, P);
        case formula::operation_t::eq_zero:
            assert(child_t_.left);
            return child_t_.left->evaluate(evals, 2 * R + P).none();
        case formula::operation_t::c:
        {
            assert(child_t_.left && child_t_.right);
            auto left = child_t_.left->evaluate(evals, 2 * R + P);
            auto right = child_t_.right->evaluate(evals, 2 * R + P);

            // all reflexive model points
            if((left & right).any())
            {
                return true;
            }

            // if there is a model point which connects left to the right
            // in the contact points which are R.
            for(int i = 0; i < 2 * R; i += 2)
            {
                if((left[i] && right[i + 1]) || (right[i] && left[i + 1]))
                {
                    return true;
                }
            }

            return false;
        }
        default:
            assert(false && "Unrecognized.");
            return false;
    }
}

std::pair<int, int> formula::get_contacts_count() const
{
    switch(op_)
    {
        case formula::operation_t::constant_true:
        case formula::operation_t::constant_false:
        case formula::operation_t::eq_zero:
            return std::pair<int, int>(0, 0);
        case formula::operation_t::negation:
        {
            assert(child_f_.left);
            std::pair<int, int> res = child_f_.left->get_contacts_count();
            return std::pair<int, int>(res.second, res.first);
        }
        case formula::operation_t::conjunction:
        case formula::operation_t::disjunction:
        {
            assert(child_f_.left);
            assert(child_f_.right);
            std::pair<int, int> left = child_f_.left->get_contacts_count();
            std::pair<int, int> right = child_f_.right->get_contacts_count();
            return std::pair<int, int>(left.first + right.first, left.second + right.second);
        }
        case formula::operation_t::c:
            return std::pair<int, int>(1, 0);
        default:
            assert(false && "Unrecognized.");
            return std::pair<int, int>(0, 0);
    }
}

std::pair<int, int> formula::get_zeroes_count() const
{
    switch(op_)
    {
        case formula::operation_t::constant_true:
        case formula::operation_t::constant_false:
        case formula::operation_t::c:
            return std::pair<int, int>(0, 0);
        case formula::operation_t::negation:
        {
            assert(child_f_.left);
            std::pair<int, int> res = child_f_.left->get_zeroes_count();
            return std::pair<int, int>(res.second, res.first);
        }
        case formula::operation_t::conjunction:
        case formula::operation_t::disjunction:
        {
            assert(child_f_.left);
            assert(child_f_.right);
            std::pair<int, int> left = child_f_.left->get_zeroes_count();
            std::pair<int, int> right = child_f_.right->get_zeroes_count();
            return std::pair<int, int>(left.first + right.first, left.second + right.second);
        }
        case formula::operation_t::eq_zero:
            return std::pair<int, int>(1, 0);
        default:
            assert(false && "Unrecognized.");
            return std::pair<int, int>(0, 0);
    }
}

void formula::clear()
{
    free();
    op_ = operation_t::invalid;
    hash_ = 0;
    child_f_ = {nullptr, nullptr};
}

auto formula::get_operation_type() const -> operation_t
{
    return op_;
}

auto formula::get_hash() const -> std::size_t
{
    return hash_;
}

auto formula::get_left_child_formula() const -> const formula*
{
    return child_f_.left;
}

auto formula::get_right_child_formula() const -> const formula*
{
    return child_f_.right;
}

auto formula::get_left_child_term() const -> const term*
{
    return child_t_.left;
}

auto formula::get_right_child_term() const -> const term*
{
    return child_t_.right;
}

auto formula::get_mgr() const -> const formula_mgr*
{
    return formula_mgr_;
}

auto formula::is_term_operation() const -> bool
{
    return op_ == operation_t::eq_zero || op_ == operation_t::c;
}

auto formula::is_atomic() const -> bool
{
    return is_term_operation();
}

auto formula::is_formula_operation() const -> bool
{
    return op_ == operation_t::conjunction || op_ == operation_t::disjunction || op_ == operation_t::negation;
}

auto formula::is_constant() const -> bool
{
    return op_ == operation_t::constant_true || op_ == operation_t::constant_false;
}

auto formula::is_constant_true() const -> bool
{
    return op_ == operation_t::constant_true;
}

auto formula::is_constant_false() const -> bool
{
    return op_ == operation_t::constant_false;
}

void formula::change_formula_mgr(formula_mgr* new_mgr)
{
    assert(new_mgr);
    formula_mgr_ = new_mgr;

    if(op_ == operation_t::eq_zero)
    {
        child_t_.left->change_formula_mgr(new_mgr);
    }
    else if(op_ == operation_t::c)
    {
        child_t_.left->change_formula_mgr(new_mgr);
        child_t_.right->change_formula_mgr(new_mgr);
    }
    else if(op_ == operation_t::negation)
    {
        child_f_.left->change_formula_mgr(new_mgr);
    }
    else if(is_formula_operation())
    {
        child_f_.left->change_formula_mgr(new_mgr);
        child_f_.right->change_formula_mgr(new_mgr);
    }
}

std::ostream& operator<<(std::ostream& out, const formula& f)
{
    switch(f.get_operation_type())
    {
        case formula::operation_t::constant_true:
            out << "T";
            break;
        case formula::operation_t::constant_false:
            out << "F";
            break;
        case formula::operation_t::conjunction:
            out << "(" << *f.get_left_child_formula() << " & " << *f.get_right_child_formula() << ")";
            break;
        case formula::operation_t::disjunction:
            out << "(" << *f.get_left_child_formula() << " | " << *f.get_right_child_formula() << ")";
            break;
        case formula::operation_t::negation:
            out << "~" << *f.get_left_child_formula();
            break;
        case formula::operation_t::eq_zero:
            out << *f.get_left_child_term() << " = 0";
            break;
        case formula::operation_t::c:
            out << "C(" << *f.get_left_child_term() << ", " << *f.get_right_child_term() << ")";
            break;
        case formula::operation_t::invalid:
            out << "UNDEFINED";
            break;
        default:
            assert(false && "Unrecognized.");
    }

    return out;
}

void formula::move(formula&& rhs) noexcept
{
    op_ = rhs.op_;
    formula_mgr_ = rhs.formula_mgr_;
    hash_ = rhs.hash_;

    if(is_term_operation())
    {
        child_t_ = rhs.child_t_;
    }
    else if(is_formula_operation())
    {
        child_f_ = rhs.child_f_;
    }

    // invalidate the rhs in order to not touch/deletes the moved resources, e.g. the childs
    rhs.op_ = operation_t::invalid;
}

auto formula::construct_eq_zero_atomic_formula(json& f) -> bool
{
    op_ = operation_t::eq_zero;

    child_t_.left = new(std::nothrow) term(formula_mgr_);
    assert(child_t_.left);

    auto& value_field = f["value"];
    if(!value_field.is_object() || !child_t_.left->build(value_field))
    {
        return false;
    }

    // add child's hashes
    hash_ = ((child_t_.left->get_hash() & 0xFFFFFFFF) * 2654435761);

    return true;
}

auto formula::construct_contact_atomic_formula(json& f) -> bool
{
    op_ = operation_t::c;

    child_t_.left = new(std::nothrow) term(formula_mgr_);
    child_t_.right = new(std::nothrow) term(formula_mgr_);
    assert(child_t_.left && child_t_.right);

    // check the json for correct information
    auto& value_field = f["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    // recursive construction of the child terms
    if(!child_t_.left->build(value_field[0]) || !child_t_.right->build(value_field[1]))
    {
        return false;
    }

    // add child's hashes and multiply be the same factor because the operation is commutative
    hash_ = ((child_t_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
            ((child_t_.right->get_hash() & 0xFFFFFFFF) * 2654435761);
    return true;
}

auto formula::construct_negation_formula(json& f) -> bool
{
    op_ = operation_t::negation;

    child_f_.left = new(std::nothrow) formula(formula_mgr_);
    assert(child_f_.left);

    auto& value_field = f["value"];

    if(!value_field.is_object() || !child_f_.left->build(value_field))
    {
        return false;
    }

    hash_ = (child_f_.left->get_hash() & 0xFFFFFFFF) * 2654435761;
    return true;
}

auto formula::construct_binary_formula(json& f, operation_t op) -> bool
{
    op_ = op;
    assert(op_ == operation_t::conjunction || op_ == operation_t::disjunction);

    child_f_.left = new(std::nothrow) formula(formula_mgr_);
    child_f_.right = new(std::nothrow) formula(formula_mgr_);
    assert(child_f_.left && child_f_.right);

    // check the json for correct information
    auto& value_field = f["value"];
    if(!value_field.is_array() || value_field.size() != 2)
    {
        return false;
    }

    // recursive construction of the child terms
    if(!child_f_.left->build(value_field[0]) || !child_f_.right->build(value_field[1]))
    {
        return false;
    }

    // add child's hashes
    hash_ = ((child_f_.left->get_hash() & 0xFFFFFFFF) * 2654435761) +
            ((child_f_.right->get_hash() & 0xFFFFFFFF) * 2654435741);
    return true;
}

void formula::free()
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
    else if(is_formula_operation())
    {
        delete child_f_.left;
        delete child_f_.right;
    }
}
