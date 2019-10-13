#include "basic_bruteforce_model.h"

auto basic_bruteforce_model::get_variables_evaluations() const -> const variable_id_to_points_t &
{
	return variable_evaluations_;
}

auto basic_bruteforce_model::get_contact_relations() const -> const contacts_t &
{
	return contact_relations_;
}


auto basic_bruteforce_model::generate_next(variables_evaluations_t& current) const -> bool
{
	for (int i = 0; i < current.size(); ++i)
	{
		current[i].flip();

		if (current[i])
		{
			return true;
		}
	}

	return false;
}

auto basic_bruteforce_model::generate_next(std::vector<variables_evaluations_t>& current) const -> bool
{
	for (int i = current.size() - 1; i >= 0; --i)
	{
		if (generate_next((current[i])))
		{	
			return true;
		}
	}
	return false;
}

auto basic_bruteforce_model::create(const formula& f, size_t variables_count) -> bool
{
	number_of_contacts_ = f.get_contacts_count().first;
	number_of_non_empty_ = f.get_zeroes_count().first;
	number_of_points_ = 2 * number_of_contacts_ + number_of_non_empty_;

	// populate 00..00 for every variable
	variable_evaluations_.clear();
	variable_evaluations_.resize(variables_count,
		variables_evaluations_t(number_of_points_, false));

	do
	{
		if (f.evaluate(variable_evaluations_, number_of_contacts_, number_of_non_empty_))
		{
			fill_contact_relations();
			return true;
		}
	} while (generate_next(variable_evaluations_));

	return false;
}

void basic_bruteforce_model::fill_contact_relations()
{
	contact_relations_.clear();
	contact_relations_.resize(number_of_points_, model_points_set_t(number_of_points_)); // Fill NxN matrix with 0s
	for (size_t i = 0; i < number_of_contacts_; i += 2)
	{
		contact_relations_[i + 1].set(i);
		contact_relations_[i].set(i + 1);
	}

	// Add also the reflexivity
	for (size_t i = 0; i < number_of_points_; ++i)
	{
		contact_relations_[i].set(i);
	}
}