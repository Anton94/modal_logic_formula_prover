#pragma once

#include "../private/linear_algebra/system_of_inequalities.h" // TODO: not pretty, refactor
#include "../private/variables_evaluations_block.h"
#include "types.h"
#include "imodel.h"

#include <memory>

/*
 * A weight should be assigned to each modal point. The measured atomic formulas specify the restrictions regarding the weight.
 * The weight is a positive real number(R+).
 *
 * The measured atomic formulas produce a system of inequalities.
 * For example, let measured_less_eq_T_ = {x <=m y ; z <=m x} measured_less_eq_F_ = {z <=m y}
 * Let SUM v(x) = SUM val(p) for all p in v(x). Where val(p) is the weight assigned to the modal point p which is part of the evaluation of the term 'x'.
 *
 * | SUM v(x) <= SUM v(y)
 * | SUM v(y) < SUM v(z) // from NOT SUM v(z) <= SUM v(y)
 * | SUM v(z) <= SUM v(x)
 * This system thoes not have a solution.
 *
 * Let a valid modal point be a point which preserves the satisfaction of the zero terms and the reflexivity of non-contacts.
 * Let a valid relation between valid modal points be a relation which preserves the satisfaction of the non-contacts.
 *
 * We can't just create a normal model and then add more modal points to satisfy the measured atomic formulas
 * because the modal points from the created model might make the measured atomic formulas unsatisfiable.
 *
 * We can't also take all valid modal points and connections between them and use them as a model
 * because there might be extra points which breakes the system which otherwise could be satisfied.
 *
 * Let us go though a couple of examples to verify the above statements.
 *
 * Let us have the variables x and y. Let the valid modal points be p1 = 00 p2 = 01 p3 = 10 p4 = 11.
 * Let W = {p1, p2, p3, p4}. Then v(x) = {p3, p4} ; v(y) = {p2, p4} ; v(x v y) = {p2, p3, p4}.
 * Let us have the following system:
 * | SUM v(x)     <= SUM v(x v y)
 * | SUM v(x v y) <= SUM v(x)
 * =>
 * | SUM {p3, p4}     <= SUM {p2, p3, p4}
 * | SUM {p2, p3, p4} <= SUM {p3, p4}
 * =>
 * | 0       <= val(p2)
 * | val(p2) <= 0
 * Contradiction, val(p2) should be positive number, could not be zero.
 * So, the model with all valid modal points thoes not have satisfiable system.
 *
 * Let W = {p3}. Thus v(x) = {p3} ; v(y) = {} ; v(x v y) = {p3}
 * | SUM {p3} <= SUM {p3}
 * | SUM {p3} <= SUM {p3}
 * Any value of R+ could be assigned to p3 and the system do have a solution.
 *
 * Let W = {p2, p3}. Thus v(x) = {p3} ; v(y) = {p2} v(x v y) = {p2, p3}
 * | SUM {p3}     <= SUM {p2, p3}
 * | SUM {p2, p3} <= SUM {p3}
 * =>
 * | 0       <= val(p2)
 * | val(p2) <= 0
 * Contradiction, val(p2) should be positive number, could not be zero.
 * So, the model with W = {p2, p3} does not satisfies the system. It also could not be extended in any way in order to satisfy it.
 *
 * The bummer is the LESS (<) inequality.
 * It requires an existence of point in the right side but this point might participate in other inequalities's left sides
 * which could break their satisfiabilities.
 *
 * THE ALGORITHM:
 * A very heavy algorithm.
 *
 * Let W be the set of all valid modal points.
 * Let R be the set of all valid relations between the points in W.
 * Let P(W) be the powerset of W, i.e. the set of all subsets of W.
 * Let W' be an element of P(W).Let R' be the set of all valid relations between the points in W'.
 * If W' and R' produces a valid model (satisfies the non-zero and contacts, the non-contacts and zero terms are already satisfied because the points and realtions are 'valid')
 * If the system is satisfied, then (W', R') is also a measured model.
 *
 * There are 2^n modal points. There are 2^(2^n) subset of modal points. Let K be the complexity for checking if a subset of modal points is a measured model
 * So the algorithm have a complexity of O(2^(2^n) * K)
 *
 * We will make a few optimization to shrink K as much as posible.
 * The valid modal points will be caluclated once. The valid connections between them will be calculated also once.
 * The evaluations of contact terms and non-zero terms will be precalculated - which valid points participate in them.
 * That way the bitwise & operation will be used for checking for existance of point in term's evaluation and relation between terms.
 *
 * There is another optimization - if a subset of points W' is not a valid model (NOT measured model, just model),
 * then there is no subset of W' which is a model. That way a lot of subset could be elminated.
 *
 * IMPLEMENTATION.
 *
 * 1) Generate all valid modal points. (only unique modal points, ofcourse)
 * 2) Generate all valid relations between them.
 * 3) Cache non-zero term's evaluations. I.e. which modal points participates in the evaluations of the contact terms and non-zero terms.
 *      This will help a lot faster checking if the subset of modal points satisfies the non-zero terms.
 *      If any of it's points is part of the evalution of the non-zero term.
 *      The evaluation is a set of points, as is the subset of points.
 *      The evaluation with only the subset of points is just an bitwise & operation over the set. A constant operation. And then checking that it's not empty. Trivial.
 * 4) Analogous for the contact terms.
 * 5) Analogous for the measured term's evaluations. Thay are needed for the system construction.
 * 6) Recursively:
 *    Generate a subset W of N valid modal points and check if it's a model:
 *      -) YES - then check if it the system has a solution:
 *          *) YES - then W and all valid relations between them are the measured model. Done!
 *          *) NO  - then continue with a subset of points of W.
 *      -) NO - then continue with another subset with N valid modal points.
 *    If there is no other subset of modal points then there is no measured model.
*/

class formula;

struct measured_model : public imodel
{
    using points_t = std::vector<variables_evaluations_block>;
    using contacts_t = std::vector<model_points_set_t>;

    measured_model(size_t max_valid_modal_points_count = 18u);

    auto create(const formulas_t& contacts_T, const formulas_t& contacts_F,
                const terms_t& zero_terms_T,  const terms_t& zero_terms_F,
                const formulas_t& measured_less_eq_T, const formulas_t& measured_less_eq_F,
                const variables_mask_t& used_variables, const variables_t& variable_names) -> bool override;

    auto print(std::ostream& out) const -> std::ostream& override;

    void clear() override;

    ~measured_model() override;

private:

    /// Recursive function which checks if @points(a subset of all valid modal points) is a measured model.
    /// If it's not it searches in all subsets of @points to find a one which is a measured model.
    /// Returns whether a measured model is generated.
    /// The generated model is recored in the member fields.
    auto generate_model(const model_points_set_t& points,
                        const formulas_t& contacts_T,
                        const formulas_t& contacts_F,
                        const terms_t& zero_terms_F,
                        const points_t& all_valid_points,
                        const contacts_t& all_valid_contact_relations,
                        const term_to_modal_points_t& term_evaluations) -> bool;

    auto create_system_of_inequalities(const model_points_set_t& points,
                                       const term_to_modal_points_t& term_evaluations,
                                       system_of_inequalities& system) const -> bool;

    void save_as_model(const model_points_set_t& points,
                       const formulas_t& contacts_F,
                       const points_t& all_valid_points);

    void calculate_the_model_evaluation_of_each_variable();

    auto print_system_sum_variables(std::ostream& out, const model_points_set_t& variables) const -> std::ostream&;
    auto print_system(std::ostream& out) const -> std::ostream&;

    size_t max_valid_modal_points_count_{};

    formulas_t measured_less_eq_T_;
    formulas_t measured_less_eq_F_;
    system_of_inequalities system_;

    std::unordered_set<model_points_set_t> processed_combinations_{};
};
