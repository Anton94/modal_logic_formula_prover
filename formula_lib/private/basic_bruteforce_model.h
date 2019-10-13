#pragma once

#include "imodel.h"
#include "types.h"
#include "formula.h"
#include "logger.h"

class basic_bruteforce_model : public i_model
{
public:
	auto get_variables_evaluations() const -> const variable_id_to_points_t & override;
	auto get_contact_relations() const -> const contacts_t & override;


	auto create(const formula& f, size_t variables_count) -> bool;

private:
	auto generate_next(variables_evaluations_t& current) const -> bool;
	auto generate_next(std::vector<variables_evaluations_t>& current) const -> bool;
	void basic_bruteforce_model::fill_contact_relations();

private:
	size_t number_of_contacts_;
	size_t number_of_non_empty_;
	size_t number_of_points_;

	variable_id_to_points_t variable_evaluations_;
	contacts_t contact_relations_;
};