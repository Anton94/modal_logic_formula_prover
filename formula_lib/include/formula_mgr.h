#pragma once

// TODO: pimpl ideom. will hide the private includes, etc.

#include "../private/formula.h"
#include "../private/tableau.h"
#include "../private/types.h"

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class formula_mgr
{
    using json = nlohmann::json;

public:
    formula_mgr();
    formula_mgr(const formula_mgr&) = delete;
    formula_mgr& operator=(const formula_mgr&) = delete;
    formula_mgr(formula_mgr&& rhs) noexcept;
    formula_mgr& operator=(formula_mgr&& rhs) noexcept;

    auto build(json& f) -> bool;

    // Generates all sequences of 0/1 to evaluate the variables and checks if the formula is satisfied
    // O(2^n) where n is the number of different variables
    auto brute_force_evaluate(variable_to_evaluation_map_t& out_evaluations) const -> bool;

	// This is the Native alghorithm which uses Sets for variables and Relations between their elements.
	// Process:
	//     For each possible Set of Relations
	//	      generate all possible variables and verify if the formula is satisfiable
	// Complexity O(2^2n)
	auto brute_force_evaluate_native(variable_to_sets_evaluation_map_t& out_evaluations) const -> bool;
	auto native_bruteforce_recursive(relations_t relations, variable_to_sets_evaluation_map_t& out_evaluations, int W, int start, int end) const -> bool;
	auto native_bruteforce_step(relations_t relations, variable_to_sets_evaluation_map_t& out_evaluations, int W) const -> bool;
	variables_evaluations_t* generate_next(variables_evaluations_t* current, int W) const;
	std::vector<variable_evaluation_set> transform_to_sets(variables_evaluations_t* bin_representation, int W) const;

	// Bruteforce algorithm without the knowledge of relations
	// Process: 
	//		Go through the formula and calculate:
	//			- R - number of contacts in the formula
	//			- P - number of subsets in the formula
	//		For all possible variablses verify if the formula is satisfiable.
	auto brute_force_evaluate_with_points_count(variable_to_bits_evaluation_map_t& out_evaluations) const -> bool;
	std::vector<variables_evaluations_t>* generate_next(std::vector<variables_evaluations_t>* current, int W) const;

	// Checks if the formula is satisfiable or not
    auto is_satisfiable() -> bool;

    // Checks if the formula evaluates to the constant true or not with the given subset of variable evaluations
    //auto does_evaluates_to_true(const variable_to_evaluation_map_t& evaluations) -> bool;

    void clear();

    auto get_variables() const -> const variables_t&;

    /// Internal API
    auto get_variable(variable_id_t id) const -> std::string;
    auto get_variable(const std::string& name) const -> variable_id_t;
    auto get_internal_formula() const -> const formula*;

    auto print(std::ostream& out, const variables_evaluations_block& block) const ->std::ostream&;
    auto print(std::ostream& out, const variables_mask_t& variables_mask) const->std::ostream&;

private:
    void move(formula_mgr&& rhs) noexcept;

    auto has_satisfiable_evaluation(const formula& f, full_variables_evaluations_t& evaluations,
                                    full_variables_evaluations_t::iterator it) const -> bool;
    auto change_variables_to_variable_ids(json& f) const -> bool;

    using variable_to_id_map_t = std::unordered_map<std::string, variable_id_t>;
    variable_to_id_map_t variable_to_id_;

    variables_t variables_;
    formula f_;
    tableau t_;
};

std::ostream& operator<<(std::ostream& out, const formula_mgr& formulas);


// Example of brute_force_evaluate_native
// The W is chosen depending on the number of variables involved in the formula 
// and probably the number of operations.
// For example:
// Let V = {a, b}, then we might choose W = {1, 2, 3}
// Here the possible relations are { <1,2>, <2,3>, <1,3> }
// meaning that we need to check for each subset of the above possible relations.
// { }

// { <1,2> }
// { <1,3> }
// { <2,3> }

// { <1,2>, <1,3> }
// { <1,2>, <2,3> }
// { <1,3>, <2,3> }

// { <1,2>, <1,3>, <2, 3> }

// For each of the above possibilities 
// we create all possible sets for the two variables
// for example
// a = {}, b = {}			- and check if satisfiable
// a = {}, b = {1}			- and check if satisfiable
// a = {}, b = {2}			- and check if satisfiable
// a = {}, b = {3}			- and check if satisfiable
// a = {}, b = {1,2}		- and check if satisfiable
// a = {}, b = {1,3}		- and check if satisfiable
// a = {}, b = {2,3}		- and check if satisfiable
// a = {}, b = {1,2,3}		- and check if satisfiable
// a = {1}, b = {}			- and check if satisfiable
// a = {1}, b = {1}			- and check if satisfiable
// ... and so on.