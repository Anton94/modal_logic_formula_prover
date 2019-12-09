#pragma once

#include "imodel.h"
#include "types.h"
#include "../private/variables_evaluations_block.h"

class formula_mgr;
class term;
class formula;

struct model : public imodel
{
    /*
     *  Les't have 3 contacts and 1 non-zero term in the following formula:
     *
     *  C(a,b) & C(c,d) & C(e,f) & g != 0 & x = 0 & ~C(y,z)
     *
     *  The model is the following:
     *
     *  (xxx...x) (a) O--------------1 (b) (xxx...x)
     *  (xxx...x) (c) 2--------------3 (d) (xxx...x)
     *  (xxx...x) (e) 4--------------5 (f) (xxx...x)
     *  (xxx...x) (g) 6
     *  where (xxx...x) is a bitset, e.g. (010...1) which gives 0/1 evaluation for the variables in the
     *  formula, so its size is the number of different variables in the formula
     *  i = 0...6 are the model points (reflexive), the contact points are connected (-------) (also symetric)
     *
     *  Note that the zero terms and negation of C are not interested to us because they does not require existence.
     *
     *  Model evaluation 'v' which for a given term returns a subset of the model points.
     *   - v(p) = { i | (i) (xxx...x)[p] == 1, i.e. in point 'i' the evaluation of the variable 'p' is 1 },
     *  where 'p' is a variable(an id)
     *   - v(a * b) = v(a) & v(b)
     *   - v(a + b) = v(a) | v(b)
     *   - v(-a) = ~v(a)
     *
     *  Note that each point's evaluation evaluates it's term to the constant true in order to that point to be
     *  in the MODEL evaluation of the term,
     *  i.e. for the point 0, the term 'a' and it's evaluation (xxx..x): a->evaluate(xxx..x) = constant_true
     *  in order to 0 belongs to v(a)
     *
     *  Let's have C(a,b); C(c,d); e=0; f=0; ~C(g,h); ~C(i, j)
     *  Then the model should be of the following type:
     *  (P0 a)----(P1 b)
     *  (P2 c)----(P3 d)
     *
     *  NOTE that if Pi evaluates the term 't' to constant true then the model evaluation v(t) contains the
     *  point (Pi _) (and vice versa)
     *
     *  Where P0 evaluates(binary) a to constant true && e and f to constant false
     *        P1 evaluates(binary) b to constant true && e and f to constant false
     *        P2 evaluates(binary) c to constant true && e and f to constant false
     *        P3 evaluates(binary) d to constant true && e and f to constant false
     *
     *  Also, to satisfy the requirement for ~C: (we need to check each contact relation):
     *  ~[(P0 evaluates g to constant true && P1 evaluates h to constant true) ||    // i.e. (P0 a) belongs to v(g) and (P1 b) belongs to v(h)
     *    (P1 evaluates g to constant true && P0 evaluates h to constant true)] &&   // i.e. (P1 b) belongs to v(g) and (P0 a) belongs to v(h)
     *  ~[(P0 evaluates i to constant true && P1 evaluates j to constant true) ||
     *    (P1 evaluates i to constant true && P0 evaluates j to constant true)] && ... analogous for P2 and P3.
     *
     *  The contact relation is reflexive, so for each point we also need to check also:
     *  ~(Pi evaluates g to constant true && Pi evaluates h to constant true) && // i.e. (Pi t) point is not in
     *  the model evaluation v(g) and v(h) (it's not a common point)
     *  ~(Pi evaluates i to constant true && Pi evaluates j to constant true)
     *
     *  Let P be the number of points,
     *  K - the number of atomic contact formulas, M - the number of atomic ~ contact formulas,
     *  N - the number of variables in the formula, L - the maximal number of logical symbols in a term,
     *  G - the number of =0 atomicformulas, H - the number of !=0 atomic formulas
     *  The evaluation of a term is with a avrg. complexity of O(L) (gives 0/1 result) which is not included in O notations bellow.
     *
     *  The algorithm which creates such points in some iterative manner is:
     *
     *  1) Creates contact points, i.e. for each C(a,b) we create two points Pa and Pb. Note that a point is just an variable evaluation.
     *  They satsfy the following rules:
     *    - Pa[a]=1 and Pb[b]=1.
     *    - does not break any t=0 atomic formula, i.e. Pa[t]=0 and Pb[t]=0.
     *    - does not break the reflexivity part of all ~C(e,f) atomic formulas, i.e. Pa[e]=0 & Pa[f]=0 & Pb[e]=0 & Pb[f]=0.
     *    - does not break any ~C(e,f) atomic formulas, i.e. ~[(Pa[e]=1 & Pb[f]=1) | (Pb[e]=1 & Pa[f]=1)]
     *  If any of the rules above is not satisfied we change Pb's evaluation, if not possible to generate a new evaluation for Pb then we change Pa's evaluation and reset the Pb's evaluatuion to the 'first' one.
     *  If again some rules is broken we change Pb's evaluation, etc. if we can't generate new evaluation for Pa then there is no satisfiable model.
     *  O(2^N * 2^N * (G+M))
     *
     *  2) Creates a point for each a!=0 atomic formula - Pa, which satisfy the following rules:
     *    - Pa[a] = 1
     *    - does not break any t=0 atomic formula, i.e. Pa[t]=0
     *    - does not break the reflexivity part of all ~C(e,f) atomic formulas, i.e. Pa[e]=0 & Pa[f]=0
     *  If any of the rules above is not satsfied with Pa, then we generate next Pa's evaluation and check them again. If not possible to generate next evaluation then there is no satisfiable model.
     *  O(2^N * (G+M))
     *
     *  3) We create the contact connectivity matrix - PxP in a simple way because we know that only first 2K points are in contact.
     *  0 <= i < K: set a contact relation between point i and i+1, i.e. we set the bit [i][i+1] to 1 and [i+1][i] to 1.
     *  O(P^2)
     *
     *  5) We calculate the v(X) for each variable.
     *  We create a vector of N bitsets. Each bitset will hold all points in the evaluation of that variable, i.e. bitset at position 'j' is the set v(j), where 'j' is some variable id.
     *  For each point Pi, we iterate over the set bits in it's evaluation and if we have a set bit at position j we add that point to the evaluation of varialbe 'j', i.e. we set the bit 'i' in the variable's evaluation bitset.
     *  O(P * N)
     *
     *  Overall: O(2^N * 2^N * (G+M)) (I suppose that 2^N >>> P)
     *
     */

    using points_t = std::vector<variables_evaluations_block>; // TODO: consider moving it to imodel

    auto create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
                const terms_t& zero_terms_F, const formulas_t& measured_less_eq_T, const formulas_t& measured_less_eq_F, const variables_mask_t& used_variables, const formula_mgr* mgr)
        -> bool override;

    auto get_model_points() const -> const points_t&;

    auto print(std::ostream& out) const -> std::ostream& override;

    void clear() override;

    ~model() override = default;

private:
    auto construct_contact_model_points(const formulas_t& contacts_T, const formulas_t& contacts_F,
                                        const terms_t& zero_terms_T) -> bool;
    auto construct_non_zero_model_points(const terms_t& zero_terms_F, const formulas_t& contacts_F,
                                         const terms_t& zero_terms_T) -> bool;

    /*
     * Creates a point evaluation which evaluates the term 't' to true,
     * evaluates all zero_terms_T terms to false and
     * for each ~C(a,b) the following is satisfied: !(evaluates a to true && evaluates b to true), i.e. the
     * reflexivity of the point and the ~C
     */
    auto create_point_evaluation(const term* t, variables_evaluations_block& out_evaluation,
                                 const formulas_t& contacts_F, const terms_t& zero_terms_T) const -> bool;
    /*
     * Generates the next point evaluation which evaluates the term 't' to true,
     * evaluates all zero_terms_T terms to false and
     * for each ~C(a,b) the following is satisfied: !(evaluates a to true && evaluates b to true), i.e. the
     * reflexivity of the point and the ~C
     */
    auto generate_next_point_evaluation(const term* t, variables_evaluations_block& out_evaluation,
                                        const formulas_t& contacts_F, const terms_t& zero_terms_T) const
        -> bool;

    /*
     * For the contact 'c' it creates two points for it's left child term (L) and right child term (R) via
     * create_point_evaluation method
     * let's call them EL and ER (evaluation left/right)
     *
     * Also, for each ~C(a,b) the following is satisfied:
     *  !(EL evaluates a to constant true && ER evaluates b to constant true)
     * i.e. ![(EL L) belong to v(a) && (ER R) belong to v(b)] which will broke ~C(a,b)
     */
    auto construct_contact_points(const formula* c, const formulas_t& contacts_F, const terms_t& zero_terms_T)
        -> bool;

    /*
     * Returns true if evaluation evaluates all zero terms to false
     */
    auto are_zero_terms_T_satisfied(const terms_t& zero_terms_T,
                                    const variables_evaluations_block& evaluation) const -> bool;
    /*
     * Returns true if for each ~C(a,b) the following is satisfied:
     *  !(evaluation evaluates a to true && evaluation evaluates b to true), i.e. the reflexivity of
     * the point and the ~C
     */
    auto is_contacts_F_reflexive_rule_satisfied(const formulas_t& contacts_F,
                                                const variables_evaluations_block& evaluation) const -> bool;

    /*
     * Returns true if the evaluation evaluates the term 't' to true,
     * are_zero_terms_T_satisfied & is_contacts_F_reflexive_rule_satisfied also return true
     */
    auto does_point_evaluation_satisfies_basic_rules(const term* t,
                                                     const variables_evaluations_block& evaluation,
                                                     const formulas_t& contacts_F,
                                                     const terms_t& zero_terms_T) const -> bool;
    /*
     * Returns true if for each ~C(a,b) the following is satisfied:
     *  !(evaluation_left evaluates a to true && evaluation_right evaluates b to true)
     */
    auto is_contacts_F_connectivity_rule_satisfied(const formulas_t& contacts_F,
                                                   const variables_evaluations_block& eval_a,
                                                   const variables_evaluations_block& eval_b) const
        -> bool;

    void calculate_the_model_evaluation_of_each_variable();

    variables_mask_t used_variables_{};

    points_t points_;
};
