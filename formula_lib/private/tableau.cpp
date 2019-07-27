#include "tableau.h"
#include "formula.h"
#include "term.h"
#include "logger.h"

auto tableau::is_satisfiable(const formula_mgr& f) -> bool
{
    clear();

    info() << "Running a satisfiability checking of " << f;

    const auto internal_f = f.get_internal_formula();
    if(internal_f->is_constant())
    {
        return internal_f->get_operation_type() == formula::operation_t::constant_true ? true : false;
    }
    add_formula_to_T(internal_f);

    return step();
}

void tableau::clear()
{
    formulas_T_.clear();
    formulas_F_.clear();
    contacts_T_.clear();
    contacts_F_.clear();
    zero_terms_T_.clear();
    zero_terms_F_.clear();
}

auto tableau::step() -> bool
{
    using op_t = formula::operation_t;

    trace() << "Making an algorithm step:";
    log_state();

    if(formulas_T_.empty() && formulas_F_.empty())
    {
        // TODO: expand the terms make the checks for contradictions in them
        trace() << "There is no contradiciton in the path";
        return true;
    }

    if(!formulas_T_.empty())
    {
        // choosing some formula to handle in this step. Here we can choose in a smarter way (TODO)
        auto f = *formulas_T_.begin();

        formulas_T_.erase(formulas_T_.begin());
        trace() << "Process " << *f << " from T formulas";
        log_state();

        const auto op = f->get_operation_type();
        if(op == op_t::negation)
        {
            // T(~X) -> F(X)
            auto not_negated_f = f->get_left_child_formula();
            if (not_negated_f->is_constant())
            {
                // F(T) is not satisfiable
                if (not_negated_f->get_operation_type() == op_t::constant_true)
                {
                    trace() << "Found a contradiction - found " << *f << " constant in T formulas";
                    return false;
                }
                return true;
            }

            if(find_in_T(not_negated_f))
            {
                trace() << "Found a contradiction - " << *not_negated_f << " found in T formulas";
                return false;
            }

            if (has_broken_contact_rule_F(not_negated_f)) // T(~C(a,b)) -> F(C(a,b))
            {
                return false;
            }
            add_formula_to_F(not_negated_f);
            auto res = step();
            remove_formula_from_F(not_negated_f);
            return res;
        }

        if(op == op_t::conjunction)
        {
            // T(X & Y) -> T(X) & T(Y)
            auto left_f = f->get_left_child_formula();
            auto right_f = f->get_right_child_formula();

            const auto left_f_op = left_f->get_operation_type();
            const auto right_f_op = right_f->get_operation_type();
            // T(F) is not satisfiable
            if (left_f_op == op_t::constant_false || right_f_op == op_t::constant_false)
            {
                trace() << "Found a contradiction - " << *f << " has a constant F as a child";
                return false;
            }

            auto process_T_conj_child = [&](const formula* child)
            {
                if (child->is_constant())
                {
                    return true;
                }
                if (find_in_F(child))
                {
                    trace() << "Found a contradiction with child " << *child
                        << " which has been found in F";
                    return false;
                }
                if (has_broken_contact_rule_T(child))
                {
                    return false;
                }
                add_formula_to_T(child);
                return true;
            };
            if (!process_T_conj_child(left_f) || !process_T_conj_child(right_f)) // TODO: what if right child fails, then I need to remove the left child from the formulas.. FIX IT
            {
                return false;
            }

            auto res = step();

            remove_formula_from_T(left_f);
            remove_formula_from_T(right_f);

            return res;
        }

        assert(op == op_t::disjunction);

        // T(X v Y) -> T(X) v T(Y)
        auto left_f = f->get_left_child_formula();
        auto right_f = f->get_right_child_formula();

        trace() << "Will split to two subtrees: " << *left_f << " and " << *right_f;

        const auto left_f_op = left_f->get_operation_type();
        const auto right_f_op = right_f->get_operation_type();
        // T(T) is satisfiable and we can skip the other branch
        if (left_f_op == op_t::constant_true || right_f_op == op_t::constant_true)
        {
            return true;
        }

        // T(F) is not satisfiable
        if(left_f_op != op_t::constant_false && !find_in_F(left_f) && !has_broken_contact_rule_T(left_f))
        {
            add_formula_to_T(left_f);

            if(step())
            {
                // there was no contradiction in that path, so there is no need to continue with the right
                // path
                return true;
            }
            remove_formula_from_T(left_f);
        }

        if(right_f_op != op_t::constant_false && !find_in_F(right_f) && !has_broken_contact_rule_T(right_f))
        {
            add_formula_to_T(right_f);
            if(step())
            {
                return true;
            }
            remove_formula_from_T(right_f);
        }
        return false;
    }

    // almost analogous but taking a formula from Fs
    assert(!formulas_F_.empty());

    // choosing some formula to handle in this step. Here we can choose in a smarter way (TODO)
    auto f = *formulas_F_.begin();
    formulas_F_.erase(formulas_F_.begin());
    trace() << "Process " << *f << " from F formulas";
    log_state();

    const auto op = f->get_operation_type();
    if(op == op_t::negation)
    {
        // F(~X) -> T(X)
        auto not_negated_f = f->get_left_child_formula();
        if (not_negated_f->is_constant())
        {
            // T(F) is not satisfiable
            if (not_negated_f->get_operation_type() == op_t::constant_false)
            {
                trace() << "Found a contradiction - found " << *f << " constant in F formulas";
                return false;
            }
            return true;
        }
        if(find_in_F(not_negated_f))
        {
            trace() << "Found a contradiction - " << *not_negated_f << " found in F formulas";
            return false;
        }

        if (has_broken_contact_rule_T(not_negated_f)) // F(~C(a,b)) -> T(C(a,b))
        {
            return false;
        }
        add_formula_to_T(not_negated_f);
        auto res = step();
        remove_formula_from_T(not_negated_f);
        return res;
    }

    if(op == op_t::disjunction)
    {
        // F(X v Y) -> F(X) & F(Y)
        auto left_f = f->get_left_child_formula();
        auto right_f = f->get_right_child_formula();

        const auto left_f_op = left_f->get_operation_type();
        const auto right_f_op = right_f->get_operation_type();
        // F(T) is not satisfiable
        if (left_f_op == op_t::constant_true || right_f_op == op_t::constant_true)
        {
            trace() << "Found a contradiction - " << *f << " has a constant T as a child";
            return false;
        }

        auto process_F_disj_child = [&](const formula* child)
        {
            if (child->is_atomic())
            {
                return true;
            }
            if (find_in_T(child))
            {
                trace() << "Found a contradiction with child " << *child
                    << " which has been found in T";
                return false;
            }
            if (has_broken_contact_rule_F(child))
            {
                return false;
            }
            add_formula_to_F(child);
            return true;
        };
        if (!process_F_disj_child(left_f) || process_F_disj_child(right_f))
        {
            return false;
        }

        auto res = step();

        remove_formula_from_F(left_f);
        remove_formula_from_F(right_f);

        return res;
    }

    assert(op == op_t::conjunction);

    // F(X & Y) -> F(X) v F(Y)
    auto left_f = f->get_left_child_formula();
    auto right_f = f->get_right_child_formula();

    trace() << "Will split to two subtrees: " << *left_f << " and " << *right_f;

    const auto left_f_op = left_f->get_operation_type();
    const auto right_f_op = right_f->get_operation_type();

    // F(F) is satisfiable and we can skip the other branch
    if (left_f_op == op_t::constant_false || right_f_op == op_t::constant_false)
    {
        return true;
    }

    // left branch of the path
    // F(T) is not satisfiable
    if(left_f_op != op_t::constant_true && !find_in_T(left_f) && !has_broken_contact_rule_F(left_f))
    {
        add_formula_to_F(left_f);
        if(step())
        {
            // there was no contradiction in that path, so there is no need to continue with the right path
            return true;
        }
        remove_formula_from_F(left_f);
    }

    if(right_f_op != op_t::constant_true && !find_in_T(right_f) && !has_broken_contact_rule_F(right_f))
    {
        add_formula_to_F(right_f);
        if(step())
        {
            return true;
        }
        remove_formula_from_F(right_f);
    }
    return false;
}

auto tableau::has_broken_contact_rule_T(const formula* f) const -> bool
{
    if (f->get_operation_type() == formula::operation_t::c)
    {
        // T(C(a,b)) has broken contact rule if a = 0 | b = 0
        const auto a = f->get_left_child_term();
        const auto b = f->get_right_child_term();
        if(zero_terms_T_.find(a) != zero_terms_T_.end() ||
            zero_terms_T_.find(b) != zero_terms_T_.end())
        {
            trace() << "Found a contradiction with the contact rule - T(" << *f << ") has a zero term";
            return true;
        }
    }

    return false;
}

auto tableau::has_broken_contact_rule_F(const formula* f) const -> bool
{
    if (f->get_operation_type() == formula::operation_t::c)
    {
        // F(C(a,b)) has broken contact rule if a != 0 & b != 0
        const auto a = f->get_left_child_term();
        const auto b = f->get_right_child_term();
        if (zero_terms_F_.find(a) != zero_terms_F_.end() &&
            zero_terms_F_.find(b) != zero_terms_F_.end())
        {
            trace() << "Found a contradiction with the contact rule - F(" << *f << ") 's terms are not zero";
            return true;
        }
    }
    return false;
}

auto tableau::find_in_T(const formula* f) const -> bool
{
    const auto op = f->get_operation_type();
    if (op == formula::operation_t::c)
    {
        return contacts_T_.find(f) != contacts_T_.end();
    }

    if(op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        return zero_terms_T_.find(t) != zero_terms_T_.end();
    }
    assert(f->is_formula_operation());
    return formulas_T_.find(f) != formulas_T_.end();
}

auto tableau::find_in_F(const formula* f) const -> bool
{
    const auto op = f->get_operation_type();
    if (op == formula::operation_t::c)
    {
        return contacts_F_.find(f) != contacts_F_.end();
    }

    if (op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        return zero_terms_F_.find(t) != zero_terms_F_.end();
    }
    assert(f->is_formula_operation());
    return formulas_F_.find(f) != formulas_F_.end();
}

void tableau::add_formula_to_T(const formula* f)
{
    const auto op = f->get_operation_type();
    if (op == formula::operation_t::c)
    {
        trace() << "Adding " << *f << " to T contacts";
        contacts_T_.insert(f);
    }
    else if (op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        trace() << "Adding " << *t << " to zero terms T because " << *f << " is eq_zero formula";
        zero_terms_T_.insert(t);
    }
    else if(f->is_formula_operation())
    {
        trace() << "Adding " << *f << " to T formulas";
        formulas_T_.insert(f);
    }
    else
    {
        trace() << "Skipping adding constant formula " << *f << " to T formulas";
    }
}

void tableau::add_formula_to_F(const formula* f)
{
    const auto op = f->get_operation_type();
    if (op == formula::operation_t::c)
    {
        trace() << "Adding " << *f << " to F contacts";
        contacts_F_.insert(f);
    }
    else if (op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        trace() << "Adding " << *t << " to zero terms F because " << *f << " is eq_zero formula";
        zero_terms_F_.insert(t);
    }
    else if(f->is_formula_operation())
    {
        trace() << "Adding " << *f << " to F formulas";
        formulas_F_.insert(f);
    }
    else
    {
        trace() << "Skipping adding constant formula " << *f << " to F formulas";
    }
}

void tableau::remove_formula_from_T(const formula* f)
{
    const auto op = f->get_operation_type();
    if (op == formula::operation_t::c)
    {
        trace() << "Removing " << *f << " from T contacts";
        contacts_T_.erase(f);
    }
    else if (op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        trace() << "Removing " << *t << " from zero terms T because " << *f << " is eq_zero formula";
        zero_terms_T_.erase(t);
    }
    else if(f->is_formula_operation())
    {
        trace() << "Removing " << *f << " from T formulas";
        formulas_T_.erase(f);
    }
    else
    {
        trace() << "Skipping removing constant formula " << *f << " from T formulas";
    }
}

void tableau::remove_formula_from_F(const formula* f)
{
    const auto op = f->get_operation_type();
    if (op == formula::operation_t::c)
    {
        trace() << "Removing " << *f << " from F contacts";
        contacts_F_.erase(f);
    }
    else if (op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        trace() << "Removing " << *t << " from zero terms because " << *f << " is eq_zero formula";
        zero_terms_F_.erase(t);
    }
    else if(f->is_formula_operation())
    {
        trace() << "Removing " << *f << " from F formulas";
        formulas_F_.erase(f);
    }
    else
    {
        trace() << "Skipping removing constant formula " << *f << " from F formulas";
    }
}

auto tableau::formula_ptr_hasher::operator()(const formula* const& f) const -> std::size_t
{
    assert(f);
    return f->get_hash();
}

auto tableau::formula_ptr_comparator::operator()(const formula* const& lhs, const formula* const& rhs) const
    -> bool
{
    assert(lhs && rhs);
    return *lhs == *rhs;
}

auto tableau::term_ptr_hasher::operator()(const term* const& t) const -> std::size_t
{
    assert(t);
    return t->get_hash();
}

auto tableau::term_ptr_comparator::operator()(const term* const& lhs, const term* const& rhs) const
    -> bool
{
    assert(lhs && rhs);
    return *lhs == *rhs;
}

void tableau::log_state() const
{
    trace() << "\t  Formulas T: " << formulas_T_;
    trace() << "\t  Formulas F: " << formulas_F_;
    trace() << "\t  Contacts T: " << contacts_T_;
    trace() << "\t  Contacts F: " << contacts_F_;
    trace() << "\tZero terms T: " << zero_terms_T_;
    trace() << "\tZero terms F: " << zero_terms_F_;
}

std::ostream& operator<<(std::ostream& out, const tableau::formulas_t& formulas)
{
    for(const auto f_ptr : formulas)
    {
        out << *f_ptr << " <" << f_ptr->get_hash() << "> ";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const tableau::terms_t& terms)
{
    for (const auto t_ptr : terms)
    {
        out << *t_ptr << " <" << t_ptr->get_hash() << "> ";
    }

    return out;
}
