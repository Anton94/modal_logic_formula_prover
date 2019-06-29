#include "tableau.h"

auto tableau::proof(const formula& f) -> bool
{
    clear();

    // if the negation of @f is not satisfiable, then @f is a tautology
    add_formula_to_F(&f);

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
    if (formulas_T_.empty() && formulas_F_.empty())
    {
        // TODO: fill some info for the state of formulas which are not contradicting, etc...
        // TODO: expand the terms make the checks for contradictions in them
        return false;
    }

    if (!formulas_T_.empty())
    {
        // choosing some formula to handle in this step. Here we can choose in a smarter way (TODO)
        auto f = *formulas_T_.begin();
        formulas_T_.erase(formulas_T_.begin());

        if (f->get_operation_type() == formula::operation_t::negation)
        {
            auto not_negated_f = f->left_f_;
            if (check_contradiction_in_T(not_negated_f))
            {
                return true;
            }
            add_formula_to_F(not_negated_f);
            auto res = step();
            remove_formula_from_F(not_negated_f);
            return res;
        }

        if (f->get_operation_type() == formula::operation_t::conjunction)
        {
            auto left_f = f->left_f_;
            auto right_f = f->right_f_;

            if (check_contradiction_in_F(left_f))
            {
                return true;
            }
            add_formula_to_T(left_f);

            if (check_contradiction_in_F(right_f))
            {
                return true;
            }
            add_formula_to_T(right_f);

            auto res = step();

            remove_formula_from_T(left_f);
            remove_formula_from_T(right_f);

            return res;
        }

        assert(f->get_operation_type() == formula::operation_t::disjunction);

        auto left_f = f->left_f_;
        auto right_f = f->right_f_;

        // left branch of the path
        auto res = true;
        if (!check_contradiction_in_F(left_f))
        {
            add_formula_to_T(left_f);
            res &= step();
            if (!res)
            {
                // there was no contradiction in that path, so there is no need to continue with the right path
                return false;
            }
            remove_formula_from_T(left_f);
        }

        if (!check_contradiction_in_F(right_f))
        {
            add_formula_to_T(right_f);
            res &= step();
            remove_formula_from_T(right_f);
        }
        return res;
    }

    // almost analogous but taking a formula from Fs
    assert(!formulas_F_.empty());

    // choosing some formula to handle in this step. Here we can choose in a smarter way (TODO)
    auto f = *formulas_F_.begin();
    formulas_F_.erase(formulas_F_.begin());

    if (f->get_operation_type() == formula::operation_t::negation)
    {
        auto not_negated_f = f->left_f_;
        if (check_contradiction_in_F(not_negated_f))
        {
            return true;
        }
        add_formula_to_T(not_negated_f);
        auto res = step();
        remove_formula_from_T(not_negated_f);
        return res;
    }

    if (f->get_operation_type() == formula::operation_t::disjunction)
    {
        auto left_f = f->left_f_;
        auto right_f = f->right_f_;

        if (check_contradiction_in_T(left_f))
        {
            return true;
        }
        add_formula_to_F(left_f);

        if (check_contradiction_in_T(right_f))
        {
            return true;
        }
        add_formula_to_F(right_f);

        auto res = step();

        remove_formula_from_F(left_f);
        remove_formula_from_F(right_f);

        return res;
    }

    assert(f->get_operation_type() == formula::operation_t::conjunction);

    auto left_f = f->left_f_;
    auto right_f = f->right_f_;

    // left branch of the path
    auto res = true;
    if (!check_contradiction_in_T(left_f))
    {
        add_formula_to_F(left_f);
        res &= step();
        if (!res)
        {
            // there was no contradiction in that path, so there is no need to continue with the right path
            return false;
        }
        remove_formula_from_F(left_f);
    }

    if (!check_contradiction_in_T(right_f))
    {
        add_formula_to_F(right_f);
        res &= step();
        remove_formula_from_F(right_f);
    }
    return res;

    return res;
}

auto tableau::check_contradiction_in_T(const formula* f) const -> bool
{
    if (f->is_atomic())
    {
        return atomic_formulas_T_.find(f) != atomic_formulas_T_.end();
    }
    return formulas_T_.find(f) != formulas_T_.end();
}

auto tableau::check_contradiction_in_F(const formula* f) const -> bool
{
    if (f->is_atomic())
    {
        return atomic_formulas_F_.find(f) != atomic_formulas_F_.end();
    }
    return formulas_F_.find(f) != formulas_F_.end();
}

void tableau::add_formula_to_T(const formula* f)
{
    if (f->is_atomic())
    {
        atomic_formulas_T_.insert(f);
    }
    else
    {
        formulas_T_.insert(f);
    }
}

void tableau::add_formula_to_F(const formula* f)
{
    if (f->is_atomic())
    {
        atomic_formulas_F_.insert(f);
    }
    else
    {
        formulas_F_.insert(f);
    }
}

void tableau::remove_formula_from_T(const formula* f)
{
    if (f->is_atomic())
    {
        atomic_formulas_T_.erase(f);
    }
    else
    {
        formulas_T_.erase(f);
    }
}

void tableau::remove_formula_from_F(const formula* f)
{
    if (f->is_atomic())
    {
        atomic_formulas_F_.erase(f);
    }
    else
    {
        formulas_F_.erase(f);
    }
}

auto tableau::formula_ptr_hasher::operator()(const formula* const& f) const->std::size_t
{
    assert(f);
    return f->get_hash();
}

auto tableau::formula_ptr_comparator::operator()(const formula* const& lhs, const formula* const& rhs) const -> bool
{
    assert(lhs && rhs);
    return *lhs == *rhs;
}