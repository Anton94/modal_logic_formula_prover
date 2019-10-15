#include "../include/basic_bruteforce_model.h"
#include "formula_mgr.h"

void basic_bruteforce_model::clear()
{
    number_of_contacts_ = 0;
    number_of_non_empty_ = 0;
    number_of_points_ = 0;

    imodel::clear();
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
    number_of_non_empty_ = f.get_zeroes_count().second;
    number_of_points_ = 2 * number_of_contacts_ + number_of_non_empty_;

    // populate 00..00 for every variable
    variable_evaluations_.clear();
    variable_evaluations_.resize(variables_count, variables_evaluations_t(number_of_points_, false));

    do
    {
        if(f.evaluate(variable_evaluations_, number_of_contacts_, number_of_non_empty_))
        {
            create_contact_relations_first_2k_in_contact(number_of_points_, number_of_contacts_);
            return true;
        }
    } while(generate_next(variable_evaluations_));

    return false;
}

auto basic_bruteforce_model::print(std::ostream& out) const -> std::ostream&
{
    // TODO: fill some additional info, e.g. evaluation for the points
    return out;
}
