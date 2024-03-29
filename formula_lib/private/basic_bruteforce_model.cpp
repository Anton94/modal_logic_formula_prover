#include "../include/basic_bruteforce_model.h"
#include "../include/thread_termiator.h"
#include "term.h"

#include <algorithm>

void basic_bruteforce_model::clear()
{
    number_of_contacts_ = 0;
    number_of_non_empty_ = 0;
    number_of_points_ = 0;

    imodel::clear();
}

auto basic_bruteforce_model::generate_next(variables_evaluations_t& current) const -> bool
{
    for(size_t i = 0; i < current.size(); ++i)
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
    TERMINATE_IF_NEEDED();

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

auto basic_bruteforce_model::create(const formulas_t& contacts_T, const formulas_t& contacts_F,
                                    const terms_t& zero_terms_T,  const terms_t& zero_terms_F,
                                    const formulas_t& measured_less_eq_T, const formulas_t& measured_less_eq_F,
                                    const variables_mask_t& used_variables, const variables_t& variable_names) -> bool
{
    trace() << "Start creating a bruteforce model.";

    // TODO: do it in a cleaver way!!!!

    variable_names_ = variable_names;

	number_of_contacts_ = contacts_T.size();
	number_of_non_empty_ = zero_terms_F.size();
    number_of_points_ = std::max(1ul, static_cast<unsigned long>(2 * number_of_contacts_ + number_of_non_empty_));

	// populate 00..00 for every variable
	variable_evaluations_.clear();
    variable_evaluations_.resize(used_variables.size(), variables_evaluations_t(number_of_points_, 0));

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
    number_of_contacts_ = f.get_contacts_count().first;
    number_of_non_empty_ = f.get_zeroes_count().second;
    number_of_points_ = std::max(1ul, static_cast<unsigned long>(2 * number_of_contacts_ + number_of_non_empty_));

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
