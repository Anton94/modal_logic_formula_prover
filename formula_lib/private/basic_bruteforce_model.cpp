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

auto basic_bruteforce_model::generate_next(std::vector<variables_evaluations_t>& current, const variables_mask_t& used_variables) const -> bool
{
	mgr_->terminate_if_need();
	/*if (mgr_->is_terminated())
	{
		info() << "The process was terminated in brute force's satisfiability step";
	}*/

    for(int i = current.size() - 1; i >= 0; --i)
    {	
		if (used_variables[i] == false)
		{
			continue;
		}
        if(generate_next((current[i])))
        {
            return true;
        }
    }
    return false;
}

auto basic_bruteforce_model::create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
            const terms_t& zero_terms_F, const formulas_t&, const formulas_t&, const variables_mask_t& used_variables, const formula_mgr* mgr)
    -> bool
{
    assert(mgr);
    assert(mgr->get_internal_formula());
    mgr_ = mgr;
    // TODO: do it in a cleaver way!!!!

	number_of_contacts_ = contacts_T.size();
	number_of_non_empty_ = zero_terms_F.size();
	number_of_points_ = 2 * number_of_contacts_ + number_of_non_empty_;

	// populate 00..00 for every variable
	variable_evaluations_.clear();
	variable_evaluations_.resize(mgr->get_variables().size(), variables_evaluations_t(number_of_points_, 0));

	do
	{
		if (satisfiability_check(contacts_T, contacts_F, zero_terms_T, zero_terms_F))
		{
			create_contact_relations_first_2k_in_contact(number_of_points_, number_of_contacts_);
			return true;
		}
	} while (generate_next(variable_evaluations_, used_variables));

    return false;
}

auto basic_bruteforce_model::satisfiability_check(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
	const terms_t& zero_terms_F) const -> bool
{
	for (const formula* f : contacts_T)
	{
		if (!f->evaluate(variable_evaluations_, number_of_contacts_, number_of_non_empty_))
		{
			return false;
		}
	}

	for (const formula* f : contacts_F)
	{
		if (f->evaluate(variable_evaluations_, number_of_contacts_, number_of_non_empty_))
		{
			return false;
		}
	}

	for (const term* t : zero_terms_T)
	{
		if (!t->evaluate(variable_evaluations_, number_of_points_).none())
		{
			return false;
		}
	}

	for (const term* t : zero_terms_F)
	{
		if (t->evaluate(variable_evaluations_, number_of_points_).none())
		{
			return false;
		}
	}

	return true;
}

auto basic_bruteforce_model::create(const formula& f, size_t variables_count, const formula_mgr* mgr) -> bool
{
    assert(mgr);
    mgr_ = mgr;
    number_of_contacts_ = f.get_contacts_count().first;
    number_of_non_empty_ = f.get_zeroes_count().second;
    number_of_points_ = 2 * number_of_contacts_ + number_of_non_empty_;

    // populate 00..00 for every variable
    variable_evaluations_.clear();
    variable_evaluations_.resize(variables_count, variables_evaluations_t(number_of_points_, 0));
	variables_mask_t used_variables = ~variables_mask_t(variables_count);
    do
    {
        if(f.evaluate(variable_evaluations_, number_of_contacts_, number_of_non_empty_))
        {
            create_contact_relations_first_2k_in_contact(number_of_points_, number_of_contacts_);
            return true;
        }
    } while(generate_next(variable_evaluations_, used_variables));

    return false;
}

auto basic_bruteforce_model::print(std::ostream& out) const -> std::ostream&
{
    // TODO: fill some additional info, e.g. evaluation for the points
    return out;
}
