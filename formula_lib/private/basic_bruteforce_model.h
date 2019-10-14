#pragma once

#include "formula.h"
#include "imodel.h"
#include "logger.h"
#include "types.h"

class basic_bruteforce_model : public imodel
{
public:
    auto get_variables_evaluations() const -> const variable_id_to_points_t& override;
    auto get_contact_relations() const -> const contacts_t& override;

    auto create(const formula& f, size_t variables_count) -> bool;

    ~basic_bruteforce_model() override = default;

private:
    auto generate_next(variables_evaluations_t& current) const -> bool;
    auto generate_next(std::vector<variables_evaluations_t>& current) const -> bool;
    void fill_contact_relations();

private:
    size_t number_of_contacts_;
    size_t number_of_non_empty_;
    size_t number_of_points_;

    variable_id_to_points_t variable_evaluations_;
    contacts_t contact_relations_;
};
