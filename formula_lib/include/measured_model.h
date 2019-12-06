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
     * TODO: add O complexities for the algorithm(on each step, etc.)
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
     * 1) For each C(Ai, Bi) we create two points Pa and Pb: Pa[Ai] = true and Pb[Bi] = true. (if not possible then there is no such model, at each step, if we can't create next point then there is no such model)
     * 2) For each Dk!=0 we create one point P: P[Dk] = true
     *
     * 3) If the points are less than the measured atomics*2 (or zero points) then we create dummy points which are associated to '1' constant term, just to make real points and to be able to put at least one variable in both sides of the inequalities
     *
     * Now, We have at least one point.
     *
     * 4) We make a connection between the contact points, i.e. for each 0 <= i < I we add a connection in the connectivity matrix between 2i and 2i + 1 points.
     * 5) We calculate the v(X) for each variable. (TODO: this should be explained somewhere else already)
     *
     * Now, we have satisfied all C(Ai, Bi) and Dk!=0 atomic formulas.
     *
     * 6)
     *
     *
     * The main difference between this model and 'model.h' is that this one generates new points and checks some conditions
     * and the other once generates the points in a way which satisfies the conditions and every point(or pair of points for contacts)
     * which it generates are satisfiable and there is no need to go back and regenrate them.
    */

    /*
        Les't have 3 contacts and 1 non-zero term in the following formula:
        C(a,b) & C(c,d) & C(e,f) & g != 0 & x = 0 & ~C(y,z)
        The model is the following:
        (xxx...x) (a) O--------------1 (b) (xxx...x)
        (xxx...x) (c) 2--------------3 (d) (xxx...x)
        (xxx...x) (e) 4--------------5 (f) (xxx...x)
        (xxx...x) (g) 6
        Note that the zero terms and negation of C are not interested to us because they does not require existence.
        where (xxx...x) is a bitset, e.g. (010...1) which gives 0/1 evaluation for the variables in the formula, so its size is the number of different variables in the formula
        i = 0...6 are the model points (reflexive), the contact points are connected (-------) (also symetric)
        Model evaluation 'v' which for a given term returns a subset of the model points.
        :
         - v(p) = { i | (i) (xxx...x)[p] == 1, i.e. in point 'i' the evaluation of the variable 'p' is 1 }, where 'p' is a variable(an id)
         - v(a * b) = v(a) & v(b)
         - v(a + b) = v(a) | v(b)
         - v(-a) = ~v(a)
        Not that each point's evaluation evaluates it's term to the constant true in order to that point to be in the MODEL evaluation of the term,
        i.e. for the point 0, the term 'a' and it's evaluation (xxx..x): a->evaluate(xxx..x) = constant_true in order to 0 belongs to v(a)
        Connectivity axiome:
        a != 0 & a != 1 -> C(a,a*)
        in our case, we evaluate each point's term (i.e., v(a)) and if it's not the whole set of points, we will create a contact relation with an element from v(a*)
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
