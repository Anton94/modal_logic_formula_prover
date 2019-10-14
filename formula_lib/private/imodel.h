#pragma once

#include "types.h"
#include <vector>

class imodel
{
public:
    virtual auto get_variables_evaluations() const -> const variable_id_to_points_t& = 0;
    virtual auto get_contact_relations() const -> const contacts_t& = 0;
    virtual ~imodel() = default;
};
