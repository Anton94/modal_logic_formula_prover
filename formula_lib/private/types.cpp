#include "types.h"
#include "formula.h"
#include "term.h"

std::ostream& operator<<(std::ostream& out, const variables_set_t& variables)
{
    for(auto& variable : variables)
    {
        out << variable << " ";
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const variables_t& variables)
{
    for(const auto& variable : variables)
    {
        out << variable << " ";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const human_readable_variables_evaluations_t& variables_evaluations)
{
    for(const auto& variable_evaluation : variables_evaluations)
    {
        out << variable_evaluation.first << " : " << variable_evaluation.second << "\n";
    }

    return out;
}
auto formula_ptr_hasher::operator()(const formula* const& f) const -> std::size_t
{
    assert(f);
    return f->get_hash();
}

auto formula_ptr_comparator::operator()(const formula* const& lhs, const formula* const& rhs) const -> bool
{
    assert(lhs && rhs);
    return *lhs == *rhs;
}

auto term_ptr_hasher::operator()(const term* const& t) const -> std::size_t
{
    assert(t);
    return t->get_hash();
}

auto term_ptr_comparator::operator()(const term* const& lhs, const term* const& rhs) const -> bool
{
    assert(lhs && rhs);
    return *lhs == *rhs;
}

std::ostream& operator<<(std::ostream& out, const formulas_t& formulas)
{
    for(const auto f_ptr : formulas)
    {
        out << *f_ptr << " <" << f_ptr->get_hash() << "> ";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const terms_t& terms)
{
    for(const auto t_ptr : terms)
    {
        out << *t_ptr << " <" << t_ptr->get_hash() << "> ";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const multiterms_t& terms)
{
    for(const auto t_ptr : terms)
    {
        out << *t_ptr << " <" << t_ptr->get_hash() << "> ";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const multiterm_to_formula_t& mapping)
{
    for(const auto& m : mapping)
    {
        const auto term = m.first;
        const auto contact = m.second;
        out << *term << " -> " << *contact << "\n";
    }

    return out;
}
