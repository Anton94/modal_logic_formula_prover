#pragma once

#include "../private/variables_evaluations_block_for_positive_term.h"
#include "../private/linear_algebra/system_of_inequalities.h" // TODO: not pretty, refactor
#include "types.h"
#include "imodel.h"

#include <memory>

class formula_mgr;
class formula;

struct measured_model : public imodel
{
    /*
     * TODO: add O complexities for the algorithm(on each step, etc.) The term evaluation complexity is skipped
     *
     * Let us have the following formula: C(Ai, Bi) & ~C(Ej, Fj) & Dk!=0 & Gl=0 & <=m(Hs, Os) & ~<=m(Qu, Ru) (i,j,k,l,s,u can iterate over some ranges).
     * This are all atomic formulas in some path in the tableau.
     * Let put some notation: N - number of variables, M - number of atomic formulas
     * I - number of contacts, J - number of ~C, K - number of !=0, L - number of =0, S - number of <=m and U - number of ~<=m
     *
     * For each contact C(Ai, Bi) we need two connected model points A and B: A belongs to v(Ai) and B belongs to v(Bi)
     * (or one point which participates in the evaluation of both terms, but this can be simulated by having the same evaluation for both points, i.e. it's duplicated).
     * For each Dk!=0 atomic we need one model point which participates in the evaluation.
     * For each measured atomic we need two points for the both sides of inequality(TODO: explain more here, the system, etc. (maybe just ~<=m needs two points?)
     * Note that the points can share the same evaluation and in that way we can think of them as the same point but duplicated. (A point is indetified by it's evaluation)
     *
     * Each point is a pair of term (Ai, Bi or Dk) and variable evaluation(evaluation for each unique variable in all terms).
     * Let's call it P with components 't' and 'e', i.e. P(t, e)
     * The point's evaluation (P[e] evaluates the point's term (P[t]) to 'true' because that way we can garantee that P belongs to v(P[t]).
     * For example, lets have C(a + b, c * d)
     * then a valid point for 'a+b' is P(a+b, [1000]), where [1000] is the evaluation for 'a' 'b' 'c' 'd' variables. That evaluation evaluates 'a+b' to true.
     * For 'c*d' a point could be: P('c*d', [0011]).
     * We need the point to know it's associated term because we will generate 'next' evaluation which evaluates the term to 'true'.
     *
     * 1) For each C(Ai, Bi) we create two points Pa and Pb: Pa[Ai] = true and Pb[Bi] = true. If not possible - then there is no satisfiable model.
     * 2) For each Dk!=0 we create one point P: P[Dk] = true  If not possible - then there is no satisfiable model.
     *
     * 3) If the points are less than the measured atomics*2 (or zero points) then we create dummy points which are associated to '1' constant term, just to make real points and to be able to put at least one variable in both sides of the inequalities
     *
     * Now, We have at least one point.
     *
     * 4) We make a connection between the contact points, first 2*I points are from contact's terms, i.e. for each 0 <= i < I we add a connection in the connectivity matrix between 2i and 2i + 1 points.
     *
     * 5) We calculate the v(X) for each variable. (TODO: this should be explained somewhere else already)
     *
     * Now, we have satisfied all C(Ai, Bi) and Dk!=0 atomic formulas.
     *
     * Note: If any of 6) 7) or 8) is not satisfied, then we generate new combination of points - step 9) and again checking 6),7) and 8) until we can generate new combination of points. If not possible then there is no satisfiable model.
     *
     * 6) For each Gl=0 we evaluate v(Gl) and check if it's the empty set. O(L)
     *
     * 7) For each ~C(Ej, Fj) we check if v(Ej) and v(Fj) contains a common point OR one of sets(e.g.v(Ej) contains a point which is in contact with a point in the other set.
     *  O(J * (I + J) * G)
     *        G is the complexity for "bit operator &" which is most likely something like (I+J)/2^6.
     *        I+J is the number of points.

     * 8) We check also the measured atomics: (TODO: we should have already defined why we are doing it in some theoretical part of the documentation)
     *  For each <=m(a,b) calculate v(a) and v(b), then we will create
     *  an inequality of the following type: SUM_I Xi <= SUM_J Xj, where I and J are v(a) and v(b).
     *  For each ~<=m(a,b) calculate v(a) and v(b), then we will create
     *  an inequality of the following type: SUM_I Xi > SUM_J Xj, where I and J are v(a) and v(b).
     *  Each inequality is a row in the system of inequalities. If this system has a solution, then we are good and the generated model is a satisfiable one. Finish!
     *
     * TODO: somewhere else we should explain how we solve such systems!
     *
     * 9) The generation of new combination of points (remember that a points is just a variable evaluation AND a term which should be evaluated to 'true' (in order that point to be in v(point's term))):
     * Similar to binary +1 operation but instead of just 0 and 1 states on each position we have 2^N combinations(from which some will not be valid because they will not evaluate the point's term to true)
     * For example:
     *    - binary +1: 000 + 1 = 001 + 1 = 010 + 1 = 011 + 1 = 100 ...
     *    - if we have two variables and two points(let's ignore the term for now).
     *     At first they will be only 0s. [00][00] where first [00] is the evaluation of the first point which evaluates both variables to 0.
     *     [00][00] -(next combination)-> [00][01] -> [00][10] -> [00][11] -> [01][00] -> [01][01] -..-> [01][11] -> [10][00] -..-> [11][11] which is the last combination.
     *     When we bring also the condition that the point's evaluation should evaluate the term to true then we skip all evaluations which does not satisfy it.
     *     E.g. if the variables are 'x' and 'y', first point's term is 'x', second point's term is 'y' then the valid sequence of combinations will be:
     *     [x:1 y:0, 'x'][x:0 y:1, 'y'] -> [10][11] -> [11][01] -> [11][11] and that's all.
     * Note that the generation could be done like +1 operation (from back to forth) or vise versa, doesn't matter.
     *
     * O((I+J) * G), where G is the complexity for finding a next evaluation for a point(i.e. finding a evaluation which evaluates the point's term to true), which is at most 2^N.
     * After a successfull generation we need to recalculate the v(X) for each X(variable) so it's (I+J)*N, i.e. for each point's evaluation - we go through each set bit to add that point to the set of points of v(X)(where X is the variable which bit is set) TODO: better explanation
     * Overall, all combinations are: (2^N)^(I+J)
     *
    */
    measured_model();

    struct point_info
    {
        const term* t;
        variables_evaluations_block_for_positive_term evaluation;
    };
    using points_t = std::vector<point_info>;

    using contacts_t = std::vector<model_points_set_t>;

    auto create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
                const terms_t& zero_terms_F, const formulas_t& measured_less_eq_T, const formulas_t& measured_less_eq_F, const variables_mask_t& used_variables, const formula_mgr* mgr)
        -> bool override;

    auto get_model_points() const -> const points_t&;

    auto print(std::ostream& out) const -> std::ostream& override;

    void clear() override;

    ~measured_model() override;

private:
    auto construct_contact_model_points(const formulas_t& contacts) -> bool;
    auto construct_non_zero_model_points(const terms_t& non_zero_terms) -> bool;

    // Generates new evaluation until @t evalautes to constant true with it
    auto generate_next_positive_evaluation(const term* t, variables_evaluations_block_for_positive_term& evaluation) const -> bool;

    // Generates a new model
    auto generate_next() -> bool;

    auto is_contact_F_rule_satisfied(const formulas_t& contacts_F) const -> bool;
    auto is_zero_term_rule_satisfied(const terms_t& zero_terms_T) const -> bool;
    auto has_solvable_system_of_inequalities() -> bool;

    // Return true if there is a contact between the v(a) and v(b). Note that v(X) is a set of model points and there is a contact between the two sets iff
    // exist x from v(a), exist y from v(b) and xRy.
    auto is_in_contact(const term* a, const term* b) const -> bool;
    // Returns true if v(a) is the empty set.
    auto is_empty_set(const term* a) const -> bool;

    auto is_not_in_contact(const term* a, const term* b) const -> bool;
    auto is_not_empty_set(const term* a) const -> bool;

    void calculate_the_model_evaluation_of_each_variable();

    auto print_system_sum_variables(std::ostream& out, const model_points_set_t& variables) const -> std::ostream&;
    auto print_system(std::ostream& out) const -> std::ostream&;

    friend std::ostream& operator<<(std::ostream& out, const measured_model& m);

    variables_mask_t used_variables_{};
    size_t number_of_contacts_{};

    std::unique_ptr<term> constant_true_;
    points_t points_;

    formulas_t measured_less_eq_T_;
    formulas_t measured_less_eq_F_;
    system_of_inequalities system_;
};
