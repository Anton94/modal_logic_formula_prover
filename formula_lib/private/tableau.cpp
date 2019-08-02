#include "tableau.h"
#include "../include/formula_mgr.h"
#include "formula.h"
#include "logger.h"
#include "term.h"
#include "utils.h"
#include "variables_evaluations_block_stack.h"

auto tableau::is_satisfiable(const formula& f, variables_evaluations_block& out_evaluation_block) -> bool
{
    clear();

    info() << "Running a satisfiability checking of " << f;

    if(f.is_constant())
    {
        return f.get_operation_type() == formula::operation_t::constant_true ? true : false;
    }
    add_formula_to_T(&f);

    const auto variables_count = f.get_mgr()->get_variables().size();
    block_stack_ = variables_evaluations_block_stack(variables_count);

    if(satisfiable_step())
    {
        out_evaluation_block = block_stack_.get_combined_block();
        return true;
    }
    return false;
}

void tableau::clear()
{
    formulas_T_.clear();
    formulas_F_.clear();
    contacts_T_.clear();
    contacts_F_.clear();
    zero_terms_T_.clear();
    zero_terms_F_.clear();
    contact_T_terms_.clear();
    terms_to_F_contacts_.clear();

    // TODO: block_stack_.clear();
}

auto tableau::satisfiable_step() -> bool
{
    using op_t = formula::operation_t;

    trace() << "Making an algorithm step:";
    log_state_satisfiable();

    if(formulas_T_.empty() && formulas_F_.empty())
    {
        trace() << "There is no contradiciton in the path.";
        return path_has_satisfiable_variable_evaluation();
    }

    if(!formulas_T_.empty())
    {
        // choosing some formula to handle in this step. Here we can choose in a smarter way (TODO)
        auto f = *formulas_T_.begin();
        formulas_T_.erase(formulas_T_.begin());
        trace() << "Processing " << *f << " from T formulas";
        log_state_satisfiable();

        call_on_destroy insert_processed_f_back_before_exiting([&]() {
            trace() << "Returning processed " << *f << " back to T formulas";
            formulas_T_.insert(f);
        });

        const auto op = f->get_operation_type();
        if(op == op_t::negation)
        {
            // T(~X) -> F(X)
            auto not_negated_f = f->get_left_child_formula(); // X
            if(not_negated_f->is_constant())
            {
                // F(T) is not satisfiable
                if(not_negated_f->is_constant_true())
                {
                    trace() << "Found a contradiction - found " << *f << " constant in T formulas";
                    return false;
                }
                return true; // F(F) is satisfiable
            }

            if(find_in_T(not_negated_f))
            {
                trace() << "Found a contradiction - " << *not_negated_f << " found in T formulas";
                return false;
            }

            if(has_broken_contact_rule_F(not_negated_f))
            {
                return false;
            }

            if(find_in_F(not_negated_f))
            {
                return satisfiable_step();
            }

            add_formula_to_F(not_negated_f);
            auto res = satisfiable_step();
            remove_formula_from_F(not_negated_f);
            return res;
        }

        if(op == op_t::conjunction)
        {
            // T(X & Y) -> T(X) & T(Y)
            T_conjuction_child left(*this, f->get_left_child_formula());
            T_conjuction_child right(*this, f->get_right_child_formula());

            if(!left.validate())
            {
                return false;
            }
            left.add_to_T();

            if(!right.validate())
            {
                return false;
            }
            right.add_to_T();

            auto res = satisfiable_step();
            left.remove_from_T();
            right.remove_from_T();

            return res;
        }

        assert(op == op_t::disjunction);

        // T(X v Y) -> T(X) v T(Y)
        auto left_f = f->get_left_child_formula();
        auto right_f = f->get_right_child_formula();
        trace() << "Will split to two subtrees: " << *left_f << " and " << *right_f;

        // T(T) is satisfiable and we can skip the other branch
        if(left_f->is_constant_true() || right_f->is_constant_true())
        {
            trace() << "One of the childs is constant true";
            return true;
        }

        auto process_T_disj_child = [&](const formula* child) {
            // T(F) is not satisfiable
            if(!child->is_constant_false() && !find_in_F(child) && !has_broken_contact_rule_T(child))
            {
                if(find_in_T(child))
                {
                    return satisfiable_step();
                }

                add_formula_to_T(child);
                const auto res = satisfiable_step();
                remove_formula_from_T(child);
                return res;
            }
            return false;
        };

        trace() << "Start of the left subtree: " << *left_f << " of " << *f;
        if(process_T_disj_child(left_f))
        {
            return true; // there was no contradiction in the left path, so there is no need to continue with
                         // the right path
        }

        trace() << "Start of the right subtree: " << *right_f << " of " << *f;
        return process_T_disj_child(right_f);
    }

    // almost analogous but taking a formula from Fs
    assert(!formulas_F_.empty());

    // choosing some formula to handle in this step. Here we can choose in a smarter way (TODO)
    auto f = *formulas_F_.begin();
    formulas_F_.erase(formulas_F_.begin());
    trace() << "Processing " << *f << " from F formulas";
    log_state_satisfiable();

    call_on_destroy insert_processed_f_back_before_exiting([&]() {
        trace() << "Returning processed " << *f << " back to F formulas";
        formulas_F_.insert(f);
    });

    const auto op = f->get_operation_type();
    if(op == op_t::negation)
    {
        // F(~X) -> T(X)
        auto not_negated_f = f->get_left_child_formula(); // X
        if(not_negated_f->is_constant())
        {
            // T(F) is not satisfiable
            if(not_negated_f->is_constant_false())
            {
                trace() << "Found a contradiction - found " << *f << " constant in F formulas";
                return false;
            }
            return true; // T(T) is satisfiable
        }
        if(find_in_F(not_negated_f))
        {
            trace() << "Found a contradiction - " << *not_negated_f << " found in F formulas";
            return false;
        }

        if(has_broken_contact_rule_T(not_negated_f)) // F(~C(a,b)) -> T(C(a,b))
        {
            return false;
        }

        if(find_in_T(not_negated_f))
        {
            return satisfiable_step();
        }

        add_formula_to_T(not_negated_f);
        auto res = satisfiable_step();
        remove_formula_from_T(not_negated_f);
        return res;
    }

    if(op == op_t::disjunction)
    {
        // F(X v Y) -> F(X) & F(Y)
        F_disjunction_child left(*this, f->get_left_child_formula());
        F_disjunction_child right(*this, f->get_right_child_formula());

        if(!left.validate())
        {
            return false;
        }
        left.add_to_F();

        if(!right.validate())
        {
            return false;
        }
        right.add_to_F();

        auto res = satisfiable_step();

        left.remove_from_F();
        right.remove_from_F();

        return res;
    }

    assert(op == op_t::conjunction);

    // F(X & Y) -> F(X) v F(Y)
    auto left_f = f->get_left_child_formula();
    auto right_f = f->get_right_child_formula();

    trace() << "Will split to two subtrees: " << *left_f << " and " << *right_f;

    // F(F) is satisfiable and we can skip the other branch
    if(left_f->is_constant_false() || right_f->is_constant_false())
    {
        trace() << "One of the childs is constant false";
        return true;
    }

    auto process_F_conj_child = [&](const formula* child) {
        // F(T) is not satisfiable
        if(!child->is_constant_true() && !find_in_T(child) && !has_broken_contact_rule_F(child))
        {
            if(find_in_F(child))
            {
                return satisfiable_step();
            }

            add_formula_to_F(child);
            const auto res = satisfiable_step();
            remove_formula_from_F(child);
            return res;
        }
        return false;
    };

    trace() << "Start of the left subtree: " << *left_f << " of " << *f;
    if(process_F_conj_child(left_f))
    {
        return true; // there was no contradiction in left path, so there is no need to continue with the
                     // right path
    }

    trace() << "Start of the right subtree: " << *right_f << " of " << *f;
    return process_F_conj_child(right_f);
}

auto tableau::has_broken_contact_rule_T(const formula* f) const -> bool
{
    const auto op = f->get_operation_type();
    if(op == formula::operation_t::c)
    {
        const auto a = f->get_left_child_term();
        const auto b = f->get_right_child_term();
        // C(a,b) -> a != 0 & b != 0
        // T(C(a,b)) has broken contact rule if a = 0 | b = 0
        auto check_for_zero_term_in_f = [&](const term* t) {
            if(zero_terms_T_.find(t) != zero_terms_T_.end())
            {
                trace() << "Found a contradiction with the contact rule - T(" << *f
                        << ") has a zero term: " << *t;
                return true;
            }
            return false;
        };
        return check_for_zero_term_in_f(a) || check_for_zero_term_in_f(b);
    }

    if(op == formula::operation_t::eq_zero)
    {
        const auto a = f->get_left_child_term();
        // a = 0 -> ~C(a, X)
        // a = 0 breaks the contact rule if there is T(C(x,y)) where x = a v y = a
        if(contact_T_terms_.find(a) != contact_T_terms_.end())
        {
            trace() << "Found a contradiction with the contact rule - there is a contact with a zero term "
                    << *a;
            return true;
        }
    }
    return false;
}

auto tableau::has_broken_contact_rule_F(const formula* f) const -> bool
{
    const auto op = f->get_operation_type();
    if(op == formula::operation_t::c)
    {
        // C(a,b) -> a != 0 & b != 0
        // F(C(a,b)) has broken contact rule if a != 0 & b != 0
        const auto a = f->get_left_child_term();
        const auto b = f->get_right_child_term();
        if(zero_terms_F_.find(a) != zero_terms_F_.end() && zero_terms_F_.find(b) != zero_terms_F_.end())
        {
            trace() << "Found a contradiction with the contact rule - F(" << *f << ") 's terms are not zero";
            return true;
        }
    }
    else if(op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        return has_broken_contact_rule_new_non_zero_term(t);
    }

    return false;
}

auto tableau::has_broken_contact_rule_new_non_zero_term(const term* key_t) const -> bool
{
    // a != 0, X != 0 -> C(a, X)
    // a != 0 breaks the contact rule if there is F(C(x,y)) where x/y = a & y/x != 0, i.e. x != 0 & y != 0

    auto iterpair = terms_to_F_contacts_.equal_range(key_t);
    for(auto it = iterpair.first, end = iterpair.second; it != end; ++it)
    {
        const auto t = it->first;
        const auto f = it->second; // F(C(x,y)) : f is a pointer to some C(x,y)
        assert(f->get_operation_type() == formula::operation_t::c);

        const auto left_child = f->get_left_child_term();
        const auto right_child = f->get_right_child_term();

        assert(*left_child == *t || *right_child == *t);
        // one of the childs is 't'
        const auto other_child = *left_child == *t ? right_child : left_child;

        // we know that 't' is a non-zero term.
        // if both terms of 'f' are the same term 't'
        // then the rule is broken
        if(*left_child == *right_child)
        {
            trace() << "Found a contradiction with the contact rule - F(" << *f
                    << ") 's terms are both not zero";
            return true;
        }
        // now we have to check if the other child is also non-zero,
        // then the rule will be broken
        if(zero_terms_F_.find(other_child) != zero_terms_F_.end())
        {
            trace() << "Found a contradiction with the contact rule - F(" << *f
                    << ") 's terms are both not zero";
            return true;
        }
    }
    return false;
}

auto tableau::find_in_T(const formula* f) const -> bool
{
    const auto op = f->get_operation_type();
    if(op == formula::operation_t::c)
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
    if(op == formula::operation_t::c)
    {
        return contacts_F_.find(f) != contacts_F_.end();
    }

    if(op == formula::operation_t::eq_zero)
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
    if(op == formula::operation_t::c)
    {
        trace() << "Adding " << *f << " to T contacts";
        contacts_T_.insert(f);

        const auto l = f->get_left_child_term();
        const auto r = f->get_right_child_term();
        trace() << "Adding " << *l << " to the terms of T contacts";
        contact_T_terms_.insert(l);
        trace() << "Adding " << *r << " to the terms of T contacts";
        contact_T_terms_.insert(r);
    }
    else if(op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        trace() << "Adding " << *t << " to zero terms T";
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
    if(op == formula::operation_t::c)
    {
        trace() << "Adding " << *f << " to F contacts";
        contacts_F_.insert(f);

        const auto l = f->get_left_child_term();
        const auto r = f->get_right_child_term();
        trace() << "Adding " << *l << " to the terms -> F contacts mapping";
        terms_to_F_contacts_.insert({l, f});
        trace() << "Adding " << *r << " to the terms -> F contacts mapping";
        terms_to_F_contacts_.insert({r, f});
    }
    else if(op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        trace() << "Adding " << *t << " to zero terms F";
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
    if(op == formula::operation_t::c)
    {
        trace() << "Removing " << *f << " from T contacts";
        bool erased = contacts_T_.erase(f);
        assert(erased);
        (void)erased;

        const auto l = f->get_left_child_term();
        const auto r = f->get_right_child_term();
        trace() << "Removing " << *l << " from the terms of T contacts";
        remove_term(contact_T_terms_, l);
        trace() << "Removing " << *r << " from the terms of T contacts";
        remove_term(contact_T_terms_, r);
    }
    else if(op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        trace() << "Removing " << *t << " from zero terms T";
        bool erased = zero_terms_T_.erase(t);
        assert(erased);
        (void)erased;
    }
    else if(f->is_formula_operation())
    {
        trace() << "Removing " << *f << " from T formulas";
        bool erased = formulas_T_.erase(f);
        assert(erased);
        (void)erased;
    }
    else
    {
        trace() << "Skipping removing constant formula " << *f << " from T formulas";
    }
}

void tableau::remove_formula_from_F(const formula* f)
{
    const auto op = f->get_operation_type();
    if(op == formula::operation_t::c)
    {
        trace() << "Removing " << *f << " from F contacts";
        bool erased = contacts_F_.erase(f);
        assert(erased);
        (void)erased;

        const auto l = f->get_left_child_term();
        const auto r = f->get_right_child_term();
        trace() << "Removing " << *l << " from the terms -> F contacts mapping";
        remove_term_to_formula(terms_to_F_contacts_, l, f);
        trace() << "Removing " << *r << " from the terms -> F contacts mapping";
        remove_term_to_formula(terms_to_F_contacts_, r, f);
    }
    else if(op == formula::operation_t::eq_zero)
    {
        const auto t = f->get_left_child_term();
        trace() << "Removing " << *t << " from zero terms";
        bool erased = zero_terms_F_.erase(t);
        assert(erased);
        (void)erased;
    }
    else if(f->is_formula_operation())
    {
        trace() << "Removing " << *f << " from F formulas";
        bool erased = formulas_F_.erase(f);
        assert(erased);
        (void)erased;
    }
    else
    {
        trace() << "Skipping removing constant formula " << *f << " from F formulas";
    }
}

void tableau::remove_term(multiterms_t& terms, const term* t)
{
    auto iterpair = terms.equal_range(t);

    for(auto it = iterpair.first, end = iterpair.second; it != end; ++it)
    {
        const auto found_t = *it;
        if(found_t == t) // note that it compairs the pointers
        {
            terms.erase(it);
            return;
        }
    }
    error() << "Unable to locate " << *t << " in the terms: " << terms;
    assert(false && "Unable to remove a term from mutitemrs");
}

void tableau::remove_term_to_formula(multiterm_to_formula_t& mapping, const term* t, const formula* f)
{
    auto iterpair = mapping.equal_range(t);

    for(auto it = iterpair.first, end = iterpair.second; it != end; ++it)
    {
        const auto found_t = it->first;
        const auto found_f = it->second;

        if(found_t == t) // note that it compairs the pointers
        {
            assert(found_f == f);
            (void)f;
            mapping.erase(it);
            return;
        }
    }
    error() << "Unable to locate " << *t << " in the terms: " << mapping;
    assert(false && "Unable to remove a term from mutitemr_to_formula");
}

void tableau::log_state_satisfiable() const
{
    trace() << "           Formulas T: " << formulas_T_;
    trace() << "           Formulas F: " << formulas_F_;
    trace() << "           Contacts T: " << contacts_T_;
    trace() << "           Contacts F: " << contacts_F_;
    trace() << "         Zero terms T: " << zero_terms_T_;
    trace() << "         Zero terms F: " << zero_terms_F_;
    trace() << "     T contacts terms: " << contact_T_terms_;
    trace() << "  terms to F contacts: " << terms_to_F_contacts_;
}

tableau::T_conjuction_child::T_conjuction_child(tableau& t, const formula* f)
    : t_(t)
    , f_(f)
{
    assert(f_);
}

auto tableau::T_conjuction_child::validate() const -> bool
{
    if(f_->is_constant_true()) // skip the T constant, T(T) is satisfiable
    {
        return true;
    }
    // T(F) is not satisfiable
    if(f_->is_constant_false())
    {
        trace() << "Found a contradiction - cojnuction in T formulas has a constant F as a child";
        return false;
    }
    if(t_.find_in_F(f_))
    {
        trace() << "Found a contradiction with child " << *f_ << " which has been found in F";
        return false;
    }

    return !t_.has_broken_contact_rule_T(f_);
}

void tableau::T_conjuction_child::add_to_T()
{
    if(!f_->is_constant() && !t_.find_in_T(f_))
    {
        t_.add_formula_to_T(f_);
        added_ = true;
    }
}

void tableau::T_conjuction_child::remove_from_T()
{
    if(added_)
    {
        t_.remove_formula_from_T(f_);
    }
}

tableau::F_disjunction_child::F_disjunction_child(tableau& t, const formula* f)
    : t_(t)
    , f_(f)
{
    assert(f_);
}

auto tableau::F_disjunction_child::validate() const -> bool
{
    if(f_->is_constant_false()) // skip the F constant, F(F) is satisfiable
    {
        return true;
    }
    // F(T) is not satisfiable
    if(f_->is_constant_true())
    {
        trace() << "Found a contradiction - disjunction in F formulas has a constant T as a child";
        return false;
    }
    if(t_.find_in_T(f_))
    {
        trace() << "Found a contradiction with child " << *f_ << " which has been found in T";
        return false;
    }

    return !t_.has_broken_contact_rule_F(f_);
}

void tableau::F_disjunction_child::add_to_F()
{
    if(!f_->is_constant() && !t_.find_in_F(f_))
    {
        t_.add_formula_to_F(f_);
        added_ = true;
    }
}

void tableau::F_disjunction_child::remove_from_F()
{
    if(added_)
    {
        t_.remove_formula_from_F(f_);
    }
}

auto tableau::path_has_satisfiable_variable_evaluation() -> bool
{
    trace() << "Start looking for an satisfiable evaluation of the variables.";

    return satisfiable_evaluation_step_contacts_T(contacts_T_.begin());
}

/*
    C(e, f) & ~C(e,f) & a = 0 & b != 0 . Four type of collections


*/

auto tableau::satisfiable_evaluation_step_contacts_T(formulas_t::iterator contacts_T_it) -> bool
{
    if(contacts_T_it == contacts_T_.end()) // all atomic C(e,f) has been evaluated -> e == 1 & f == 1
    {
        return satisfiable_evaluation_step_contacts_F(contacts_F_.begin());
    }


    const auto c = *contacts_T_it;
    auto left_eval_res = c->get_left_child_term()->evaluate(block_stack_.get_combined_block());
    auto right_eval_res = c->get_right_child_term()->evaluate(block_stack_.get_combined_block());
    // e[combined] - [P0, P1, ... , Pn, Q0, Q1, ... , Qk] // e[combined] is the term which is produced by evaluating e with the bombined evaluation block
    // f[combined] - [P0, P1, ... , Pn, R0, R1, ... , Rk]

    if(left_eval_res.is_constant_false() || right_eval_res.is_constant_false())
    {
        left_eval_res.free();
        right_eval_res.free();
        return false;
    }

    if(left_eval_res.is_constant_true() && right_eval_res.is_constant_true())
    {
        return satisfiable_evaluation_step_contacts_T(++contacts_T_it);
    }

    if(left_eval_res.is_term() && right_eval_res.is_constant_true())
    {
        return satisfiable_evaluation_step_contacts_T_left(contacts_T_it, std::move(left_eval_res),
                                                           std::move(right_eval_res));
    }

    if(left_eval_res.is_constant_true() && right_eval_res.is_term())
    {
        const auto res = satisfiable_evaluation_step_contacts_T_right(contacts_T_it, right_eval_res);
        right_eval_res.free();
        return res;
    }

    assert(left_eval_res.is_term() && right_eval_res.is_term());

    const auto& left_variables = left_eval_res.get()->get_variables();
    const auto& right_variables = right_eval_res.get()->get_variables();
    const auto common_variables = left_variables & right_variables;
    if(common_variables.any())
    {
        return satisfiable_evaluation_step_contacts_T_common(contacts_T_it, std::move(left_eval_res),
                                                             std::move(right_eval_res));
    }

    return satisfiable_evaluation_step_contacts_T_left(contacts_T_it, std::move(left_eval_res),
                                                       std::move(right_eval_res));
}

auto tableau::satisfiable_evaluation_step_contacts_T_common(formulas_t::iterator contacts_T_it,
                                                            term::evaluation_result&& left_eval_res,
                                                            term::evaluation_result&& right_eval_res) -> bool
{
    assert(left_eval_res.is_term() && right_eval_res.is_term());

    const std::unique_ptr<const term> left_evaluated_subterm(left_eval_res.release());
    const std::unique_ptr<const term> right_evaluated_subterm(right_eval_res.release());
    const auto common_variables =
        left_evaluated_subterm->get_variables() & right_evaluated_subterm->get_variables();
    assert(common_variables.any());

    assert((common_variables & block_stack_.get_combined_variables()).none());
    block_stack_.push(variables_evaluations_block(common_variables));

    do
    {
        auto left_eval_subres = left_evaluated_subterm->evaluate(block_stack_.get_combined_block());
        auto right_eval_subres = right_evaluated_subterm->evaluate(block_stack_.get_combined_block());

        if(left_eval_subres.is_constant_false() || right_eval_subres.is_constant_false())
        {
            left_eval_subres.free();
            right_eval_subres.free();
        }
        else
        {
            if (satisfiable_evaluation_step_contacts_T_left(contacts_T_it, std::move(left_eval_subres),
                std::move(right_eval_subres)))
            {
                return true;
            }
        }
    } while(block_stack_.generate_evaluation());

    block_stack_.pop();
    return false;
}

auto tableau::satisfiable_evaluation_step_contacts_T_left(formulas_t::iterator contacts_T_it,
                                                          term::evaluation_result&& left_eval_res,
                                                          term::evaluation_result&& right_eval_res) -> bool
{
    assert(contacts_T_it != contacts_T_.end());
    call_on_destroy free_right_eval_res_before_exiting([&]() { right_eval_res.free(); });

    if(left_eval_res.is_constant_false())
    {
        return false;
    }
    if(left_eval_res.is_constant_true())
    {
        return satisfiable_evaluation_step_contacts_T_right(contacts_T_it, right_eval_res);
    }

    assert(left_eval_res.is_term());
    const std::unique_ptr<const term> evaluated_subterm(left_eval_res.release());

    assert((evaluated_subterm->get_variables() & block_stack_.get_combined_variables()).none());
    block_stack_.push(variables_evaluations_block(evaluated_subterm->get_variables()));

    auto contacts_T_it_next = contacts_T_it;
    ++contacts_T_it_next;
    do
    {
        const auto eval_res = evaluated_subterm->evaluate(block_stack_.get_combined_block());
        assert(eval_res.is_constant());

        if(eval_res.is_constant_true() &&
           satisfiable_evaluation_step_contacts_T_right(
               contacts_T_it, right_eval_res)) // TODO: if the right term has no valid evaluation, then the
                                               // generation of evaluations for the left term will not lead to
                                               // a new "global" evaluation!. add bool argument to indicate if
                                               // it has produced own valid generation
        {
            return true;
        }
    } while(block_stack_.generate_evaluation());

    block_stack_.pop();
    return false;
}

auto tableau::satisfiable_evaluation_step_contacts_T_right(formulas_t::iterator contacts_T_it,
                                                           const term::evaluation_result& eval_res) -> bool
{
    assert(contacts_T_it != contacts_T_.end());

    if(eval_res.is_constant_false())
    {
        return false;
    }
    if(eval_res.is_constant_true())
    {
        return satisfiable_evaluation_step_contacts_T(++contacts_T_it);
    }

    assert(eval_res.is_term());
    const auto evaluated_subterm = eval_res.get();

    assert((evaluated_subterm->get_variables() & block_stack_.get_combined_variables()).none());
    block_stack_.push(variables_evaluations_block(evaluated_subterm->get_variables()));

    auto contacts_T_it_next = contacts_T_it;
    ++contacts_T_it_next;

    do
    {
        const auto eval_subres = evaluated_subterm->evaluate(block_stack_.get_combined_block());
        assert(eval_subres.is_constant());

        if(eval_subres.is_constant_true() && satisfiable_evaluation_step_contacts_T(contacts_T_it_next))
        {
            return true;
        }
    } while(block_stack_.generate_evaluation());

    block_stack_.pop();
    return false;
}

auto tableau::satisfiable_evaluation_step_contacts_F(formulas_t::iterator contacts_F_it) -> bool
{
    if(contacts_F_it == contacts_F_.end()) // all atomic ~C(e,f) has been evaluated C(e,f) = 0 -> e == 0 | f == 0
    {
        return satisfiable_evaluation_step_zero_terms_T(zero_terms_T_.begin());
    }

    const auto c = *contacts_F_it;
    auto left_eval_res = c->get_left_child_term()->evaluate(block_stack_.get_combined_block());
    auto right_eval_res = c->get_right_child_term()->evaluate(block_stack_.get_combined_block());
    call_on_destroy free_evals_before_exiting([&]() {
        left_eval_res.free();
        right_eval_res.free();
    });

    if(left_eval_res.is_constant_false() || right_eval_res.is_constant_false())
    {
        return satisfiable_evaluation_step_contacts_F(++contacts_F_it);
    }

    if(left_eval_res.is_constant_true() && right_eval_res.is_constant_true())
    {
        return false;
    }

    // TODO: check if there is a case where there are going to be not evaluated variables which are needed

    auto contacts_F_it_next = contacts_F_it;
    ++contacts_F_it_next;

    const auto step_child_term = [&](const term* t) {
        assert((t->get_variables() & block_stack_.get_combined_variables()).none());
        block_stack_.push(variables_evaluations_block(t->get_variables()));

        do
        {
            const auto eval_subres = t->evaluate(block_stack_.get_combined_block());
            assert(eval_subres.is_constant());

            if(eval_subres.is_constant_false() && satisfiable_evaluation_step_contacts_F(contacts_F_it_next))
            {
                return true;
            }
        } while(block_stack_.generate_evaluation());

        block_stack_.pop();
        return false;
    };

    return (left_eval_res.is_term() && step_child_term(left_eval_res.get())) ||
           (right_eval_res.is_term() && step_child_term(right_eval_res.get()));
}

auto tableau::satisfiable_evaluation_step_zero_terms_T(terms_t::iterator zero_terms_T_it) -> bool
{
    if(zero_terms_T_it == zero_terms_T_.end()) // all zero terms (a = 0) has been evaluated
    {
        return satisfiable_evaluation_step_zero_terms_F(zero_terms_F_.begin());
    }

    const auto t = *zero_terms_T_it;
    auto eval_res = t->evaluate(block_stack_.get_combined_block());
    if(eval_res.is_constant_false())
    {
        return satisfiable_evaluation_step_zero_terms_T(++zero_terms_T_it); // (a v ( b & c) ) & x
    }
    if(eval_res.is_constant_true())
    {
        return false;
    }
    assert(eval_res.is_term());
    const std::unique_ptr<const term> evaluated_subterm(eval_res.release());

    assert((evaluated_subterm->get_variables() & block_stack_.get_combined_variables()).none());
    block_stack_.push(variables_evaluations_block(evaluated_subterm->get_variables()));

    auto zero_terms_T_it_next = zero_terms_T_it;
    ++zero_terms_T_it_next;
    do
    {
        const auto eval_res = evaluated_subterm->evaluate(block_stack_.get_combined_block());
        assert(eval_res.is_constant());

        if(eval_res.is_constant_false() && satisfiable_evaluation_step_zero_terms_T(zero_terms_T_it_next))
        {
            return true;
        }
    } while(block_stack_.generate_evaluation());

    block_stack_.pop();
    return false;
}

auto tableau::satisfiable_evaluation_step_zero_terms_F(terms_t::iterator zero_terms_F_it) -> bool
{
    if(zero_terms_F_it == zero_terms_F_.end()) // all non-zero terms (b != 0) has been evaluated
    {
        return true;
    }

    // The goal is to evaluate the non-zero term to 1

    const auto t = *zero_terms_F_it;
    auto eval_res = t->evaluate(block_stack_.get_combined_block());
    if(eval_res.is_constant_true())
    {
        return satisfiable_evaluation_step_zero_terms_F(++zero_terms_F_it);
    }
    if(eval_res.is_constant_false())
    {
        return false;
    }

    assert(eval_res.is_term());
    const std::unique_ptr<const term> evaluated_subterm(eval_res.release());

    assert((evaluated_subterm->get_variables() & block_stack_.get_combined_variables()).none());
    block_stack_.push(variables_evaluations_block(evaluated_subterm->get_variables()));

    auto zero_terms_F_it_next = zero_terms_F_it;
    ++zero_terms_F_it_next;
    do
    {
        const auto eval_res = evaluated_subterm->evaluate(block_stack_.get_combined_block());
        assert(eval_res.is_constant());

        if(eval_res.is_constant_true() && satisfiable_evaluation_step_zero_terms_F(zero_terms_F_it_next))
        {
            return true;
        }
    } while(block_stack_.generate_evaluation());

    block_stack_.pop();
    return false;
}
