#pragma once

#include "../private/linear_algebra/system_of_inequalities.h" // TODO: not pretty, refactor
#include "../private/variables_evaluations_block_for_positive_term.h"
#include "model.h"
#include "types.h"

#include <memory>

class formula_mgr;
class formula;

struct optimized_measured_model : public model
{
    /*
     * See measured model for more info regarding the idea of the model.
     * This one finds a satisfiable model with the faster algorithm(via the 'model') and adds additional
     * points for the atomic measured formulas (one for each formula).
     * Let's the number of additonal points is A and the number used variables is N.
     * All different unique evaluations are 2^N.
     * We need a subset of size A from thouse evaluations for the additional points (we can have duplications).
     * So, all combinations of such subsets are (2^N)^A or 2^(N*A).
     *
     * Compared to the 'measured_model' where it is roufly 2^(N*(A+U)), where U is the number atomic contacts
     * * 2 + number of atomic !=0.
     * It is a sagnificant optimization.
     * Now, this measured model is 'bounded' from the number of atomic measured formulas instead of also the
     * number of atomic contacts and !=0 formulas.
     *
    */
    optimized_measured_model();

    auto create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
                const terms_t& zero_terms_F, const formulas_t& measured_less_eq_T,
                const formulas_t& measured_less_eq_F, const variables_mask_t& used_variables,
                const formula_mgr* mgr) -> bool override;

    auto print(std::ostream& out) const -> std::ostream& override;

    void clear() override;

    ~optimized_measured_model() override;

protected:
    auto generate_next_additional_points_combination(const formulas_t& contacts_F,
                                                     const terms_t& zero_terms_T) -> bool;

    // TODO this method could be shared with measured_model
    auto has_solvable_system_of_inequalities() -> bool;

    // Creates additional @points_count_to_construct points and adds them to @points_
    void construct_additional_points(const formulas_t& contacts_F, const terms_t& zero_terms_T,
                                     size_t points_count_to_construct);

    auto print_system_sum_variables(std::ostream& out, const model_points_set_t& variables) const
        -> std::ostream&;
    auto print_system(std::ostream& out) const -> std::ostream&;

    variables_evaluations_block additional_point_initial_value_;

    size_t points_count_without_additional_;
    formulas_t measured_less_eq_T_;
    formulas_t measured_less_eq_F_;
    system_of_inequalities system_;
};
