#include "../include/basic_bruteforce_model.h"
#include "formula_mgr.h"

void basic_bruteforce_model::clear()
{
    number_of_contacts_ = 0;
    number_of_non_empty_ = 0;
    number_of_points_ = 0;

    imodel::clear();
}


auto basic_bruteforce_model::get_number_of_contacts() const -> size_t
{
    return number_of_contacts_;
}

auto basic_bruteforce_model::get_number_of_non_zeros() const -> size_t
{
    return number_of_non_empty_;
}

auto basic_bruteforce_model::generate_next(variables_evaluations_t& current) const -> bool
{
    for(int i = 0; i < current.size(); ++i)
    {
        current[i].flip();

        if(current[i])
        {
            return true;
        }
    }

    return false;
}

auto basic_bruteforce_model::generate_next(std::vector<variables_evaluations_t>& current) const -> bool
{
    for(int i = current.size() - 1; i >= 0; --i)
    {
        if(generate_next((current[i])))
        {
            return true;
        }
    }
    return false;
}

auto basic_bruteforce_model::create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
            const terms_t& zero_terms_F, const variables_mask_t& used_variables, const formula_mgr* mgr)
    -> bool
{
    assert(mgr);
    assert(mgr->get_internal_formula());
    mgr_ = mgr;
    // TODO: do it in a cleaver way!!!!
    return create(*mgr_->get_internal_formula(), mgr_->get_variables().size());
}

auto basic_bruteforce_model::create(const formula& f, size_t variables_count) -> bool
{
    number_of_contacts_ = f.get_contacts_count().first;
    number_of_non_empty_ = f.get_zeroes_count().first;
    number_of_points_ = 2 * number_of_contacts_ + number_of_non_empty_;

    // populate 00..00 for every variable
    variable_evaluations_.clear();
    variable_evaluations_.resize(variables_count, variables_evaluations_t(number_of_points_, false));

    do
    {
        if(f.evaluate(variable_evaluations_, number_of_contacts_, number_of_non_empty_))
        {
            fill_contact_relations();
            return true;
        }
    } while(generate_next(variable_evaluations_));

    return false;
}

void basic_bruteforce_model::fill_contact_relations()
{
    contact_relations_.clear();
    contact_relations_.resize(number_of_points_,
                              model_points_set_t(number_of_points_)); // Fill NxN matrix with 0s
    for(size_t i = 0; i < number_of_contacts_; i += 2)
    {
        contact_relations_[i + 1].set(i);
        contact_relations_[i].set(i + 1);
    }

    // Add also the reflexivity
    for(size_t i = 0; i < number_of_points_; ++i)
    {
        contact_relations_[i].set(i);
    }
}

auto basic_bruteforce_model::print(std::ostream& out) const -> std::ostream&
{
    // TODO: fill some additional info, e.g. evaluation for the points
    return out;
}
