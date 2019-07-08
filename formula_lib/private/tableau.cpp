#include "tableau.h"
#include "formula.h"
#include "logger.h"

auto tableau::is_satisfiable(const formula_mgr& f) -> bool
{
    clear();

    info() << "Running a satisfiability checking of " << f;
    add_formula_to_T(&f.f_);

    return step();
}

void tableau::clear()
{
    formulas_T_.clear();
    formulas_F_.clear();
    atomic_formulas_T_.clear();
    atomic_formulas_F_.clear();
}

auto tableau::step() -> bool
{
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

        if(f->get_operation_type() == formula::operation_t::negation)
        {
            auto not_negated_f = f->child_f_.left;
            if(check_contradiction_in_T(not_negated_f))
            {
                trace() << "Found a contradiction - " << *not_negated_f << " found in T formulas";
                return false;
            }
            add_formula_to_F(not_negated_f);
            auto res = step();
            remove_formula_from_F(not_negated_f);
            return res;
        }

        if(f->get_operation_type() == formula::operation_t::conjunction)
        {
            auto left_f = f->child_f_.left;
            auto right_f = f->child_f_.right;

            if(check_contradiction_in_F(left_f))
            {
                trace() << "Found a contradiction with left child " << *left_f
                        << " which has been found in F";
                return false;
            }
            add_formula_to_T(left_f);

            if(check_contradiction_in_F(right_f))
            {
                trace() << "Found a contradiction with right child " << *right_f
                        << " which has been found in F";
                return false;
            }
            add_formula_to_T(right_f);

            auto res = step();

            remove_formula_from_T(left_f);
            remove_formula_from_T(right_f);

            return res;
        }

        assert(f->get_operation_type() == formula::operation_t::disjunction);

        auto left_f = f->child_f_.left;
        auto right_f = f->child_f_.right;

        trace() << "Will split to two subtrees: " << *left_f << " and " << *right_f;

        if(!check_contradiction_in_F(left_f))
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

        if(!check_contradiction_in_F(right_f))
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

    if(f->get_operation_type() == formula::operation_t::negation)
    {
        auto not_negated_f = f->child_f_.left;
        if(check_contradiction_in_F(not_negated_f))
        {
            trace() << "Found a contradiction - " << *not_negated_f << " found in F formulas";
            return false;
        }
        add_formula_to_T(not_negated_f);
        auto res = step();
        remove_formula_from_T(not_negated_f);
        return res;
    }

    if(f->get_operation_type() == formula::operation_t::disjunction)
    {
        auto left_f = f->child_f_.left;
        auto right_f = f->child_f_.right;

        if(check_contradiction_in_T(left_f))
        {
            trace() << "Found a contradiction with right child " << *left_f << " which has been found in T";
            return false;
        }
        add_formula_to_F(left_f);

        if(check_contradiction_in_T(right_f))
        {
            trace() << "Found a contradiction with right child " << *right_f << " which has been found in T";
            return false;
        }
        add_formula_to_F(right_f);

        auto res = step();

        remove_formula_from_F(left_f);
        remove_formula_from_F(right_f);

        return res;
    }

    assert(f->get_operation_type() == formula::operation_t::conjunction);

    auto left_f = f->child_f_.left;
    auto right_f = f->child_f_.right;

    trace() << "Will split to two subtrees: " << *left_f << " and " << *right_f;

    // left branch of the path
    if(!check_contradiction_in_T(left_f))
    {
        add_formula_to_F(left_f);
        if(step())
        {
            // there was no contradiction in that path, so there is no need to continue with the right path
            return true;
        }
        remove_formula_from_F(left_f);
    }

    if(!check_contradiction_in_T(right_f))
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

auto tableau::check_contradiction_in_T(const formula* f) const -> bool
{
    if(f->is_atomic())
    {
        return atomic_formulas_T_.find(f) != atomic_formulas_T_.end();
    }
    return formulas_T_.find(f) != formulas_T_.end();
}

auto tableau::check_contradiction_in_F(const formula* f) const -> bool
{
    if(f->is_atomic())
    {
        return atomic_formulas_F_.find(f) != atomic_formulas_F_.end();
    }
    return formulas_F_.find(f) != formulas_F_.end();
}

void tableau::add_formula_to_T(const formula* f)
{
    if(f->is_atomic())
    {
        trace() << "Adding " << *f << " to T atomic formulas";
        atomic_formulas_T_.insert(f);
    }
    else
    {
        trace() << "Adding " << *f << " to T formulas";
        formulas_T_.insert(f);
    }
}

void tableau::add_formula_to_F(const formula* f)
{
    if(f->is_atomic())
    {
        trace() << "Adding " << *f << " to F atomic formulas";
        atomic_formulas_F_.insert(f);
    }
    else
    {
        trace() << "Adding " << *f << " to F formulas";
        formulas_F_.insert(f);
    }
}

void tableau::remove_formula_from_T(const formula* f)
{
    if(f->is_atomic())
    {
        trace() << "Removing " << *f << " from T atomic formulas";
        atomic_formulas_T_.erase(f);
    }
    else
    {
        trace() << "Removing " << *f << " from T formulas";
        formulas_T_.erase(f);
    }
}

void tableau::remove_formula_from_F(const formula* f)
{
    if(f->is_atomic())
    {
        trace() << "Removing " << *f << " from F atomic formulas";
        atomic_formulas_F_.erase(f);
    }
    else
    {
        trace() << "Removing " << *f << " from F formulas";
        formulas_F_.erase(f);
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

void tableau::log_state() const
{
    trace() << "\tFormulas T: " << formulas_T_;
    trace() << "\tFormulas F: " << formulas_F_;
    trace() << "\tFormulas atomic T: " << atomic_formulas_T_;
    trace() << "\tFormulas atomic F: " << atomic_formulas_F_;
}

std::ostream& operator<<(std::ostream& out, const tableau::formulas_t& formulas)
{
    for(const auto f_ptr : formulas)
    {
        out << *f_ptr << " <" << f_ptr->get_hash() << "> ";
    }

    return out;
}
