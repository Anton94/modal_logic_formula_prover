#pragma once

#include "../private/formula.h"
#include "imodel.h"
#include "logger.h"
#include "types.h"

class basic_bruteforce_model : public imodel
{
public:

    auto create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
                const terms_t& zero_terms_F, const variables_mask_t& used_variables, const formula_mgr* mgr)
        -> bool override;
    auto create(const formula& f, size_t variables_count) -> bool;

    auto print(std::ostream& out) const -> std::ostream& override;

    void clear() override;

    ~basic_bruteforce_model() override = default;

private:
    auto generate_next(variables_evaluations_t& current) const -> bool;
    auto generate_next(std::vector<variables_evaluations_t>& current, const variables_mask_t& used_variables) const -> bool;

	auto satisfiability_check(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
		const terms_t& zero_terms_F) const -> bool;

    size_t number_of_contacts_;
    size_t number_of_non_empty_;
    size_t number_of_points_;
};
