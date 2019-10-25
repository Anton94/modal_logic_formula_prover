#pragma once

// TODO: pimpl ideom. will hide the private includes, etc.

#include "basic_bruteforce_model.h"
#include "imodel.h"
#include "../private/formula.h"
#include "../private/tableau.h"
#include "../private/types.h"

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

class formula_mgr
{
    using json = nlohmann::json;

public:
	using termination_callback = std::function<bool()>;

	formula_mgr(const termination_callback& c = {});
    formula_mgr(const formula_mgr&) = delete;
    formula_mgr& operator=(const formula_mgr&) = delete;
    formula_mgr(formula_mgr&& rhs) noexcept;
    formula_mgr& operator=(formula_mgr&& rhs) noexcept;

    // TODO: better explanations
    enum class formula_refiners : int32_t
    {
        none                                    = 0,
        convert_contact_less_eq_with_same_terms = 1 << 1,
        convert_disjunction_in_contact_less_eq  = 1 << 2,
        reduce_constants                        = 1 << 3,
        reduce_contacts_less_eq_with_constants  = 1 << 4,
        remove_double_negation                  = 1 << 5,
        all = convert_contact_less_eq_with_same_terms | convert_disjunction_in_contact_less_eq | reduce_constants | reduce_contacts_less_eq_with_constants | remove_double_negation
    };

    auto build(const std::string& f, const formula_refiners& refiners_flags = formula_refiners::all) -> bool;

    auto build(json& f) -> bool;

    // Bruteforce algorithm without the knowledge of relations
    // Process:
    //		Go through the formula and calculate:
    //			- R - number of contacts in the formula
    //			- P - number of subsets in the formula
    //		For all possible variablses verify if the formula is satisfiable.
    auto brute_force_evaluate_with_points_count(basic_bruteforce_model& out_model) const -> bool;

    // Checks if the formula is satisfiable or not
    auto is_satisfiable(imodel& out_model) -> bool;

    // Checks if the provided model satisfies the formula
    auto is_model_satisfiable(const imodel& model) const -> bool;

    void clear();

    auto get_variables() const -> const variables_t&;

    /// Internal API
    auto get_variable(variable_id_t id) const -> std::string;
    auto get_variable(const std::string& name) const -> variable_id_t;
    auto get_internal_formula() const -> const formula*;

    auto print(std::ostream& out, const variables_evaluations_block& block) const -> std::ostream&;
    auto print(std::ostream& out, const variables_mask_t& variables_mask) const -> std::ostream&;

	// throws an exception. TODO: make custom exception, etc...
	void terminate_if_need() const;

private:
    void move(formula_mgr&& rhs) noexcept;

    auto change_variables_to_variable_ids(json& f) const -> bool;

    auto has_flag(const formula_refiners& flags, const formula_refiners& flag) const -> bool;

    variable_to_id_map_t variable_to_id_;

    variables_t variables_;
    formula f_;
    tableau t_;

	termination_callback is_terminated_;
};

std::ostream& operator<<(std::ostream& out, const formula_mgr& formulas);
