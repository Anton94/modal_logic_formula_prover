#pragma once

#include <vector>
#include "types.h"

class i_model
{
public:
	virtual auto get_variables_evaluations() const -> const variable_id_to_points_t & = 0;
	virtual auto get_contact_relations() const -> const contacts_t & = 0;
};