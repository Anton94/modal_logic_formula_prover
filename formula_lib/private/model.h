#pragma once

#include "imodel.h"
#include "types.h"
#include "variables_evaluations_block.h"

class formula_mgr;
class term;
class formula;

struct model : public imodel
{
    /*
        Les't have 3 contacts and 1 non-zero term in the following formula:

        C(a,b) & C(c,d) & C(e,f) & g != 0 & x = 0 & ~C(y,z)

        The model is the following:

        (xxx...x) (a) O--------------1 (b) (xxx...x)
        (xxx...x) (c) 2--------------3 (d) (xxx...x)
        (xxx...x) (e) 4--------------5 (f) (xxx...x)
        (xxx...x) (g) 6

        Note that the zero terms and negation of C are not interested to us because they does not require
        existence.

        where (xxx...x) is a bitset, e.g. (010...1) which gives 0/1 evaluation for the variables in the
        formula, so its size is the number of different variables in the formula
        i = 0...6 are the model points (reflexive), the contact points are connected (-------) (also symetric)

        Model evaluation 'v' which for a given term returns a subset of the model points.
        :
         - v(p) = { i | (i) (xxx...x)[p] == 1, i.e. in point 'i' the evaluation of the variable 'p' is 1 },
        where 'p' is a variable(an id)
         - v(a * b) = v(a) & v(b)
         - v(a + b) = v(a) | v(b)
         - v(-a) = ~v(a)

        Not that each point's evaluation evaluates it's term to the constant true in order to that point to be
       in the MODEL evaluation of the term,
        i.e. for the point 0, the term 'a' and it's evaluation (xxx..x): a->evaluate(xxx..x) = constant_true
       in order to 0 belongs to v(a)
    */
    struct point_info
    {
        const term* t;
        variables_evaluations_block evaluation;
    };
    using points_t = std::vector<point_info>;

    /*
     * Let's have C(a,b); C(c,d); e=0; f=0; ~C(g,h); ~C(i, j)
     * Then the model should be of the following type:
     * (P0 a)----(P1 b)
     * (P2 c)----(P3 d)
     *
     * NOTE that if Pi evaluates the term 't' to constant true then the model evaluation v(t) contains the
     * point (Pi _) (and vice versa)
     *
     * Where P0 evaluates(binary) a to constant true && e and f to constant false
     *       P1 evaluates(binary) b to constant true && e and f to constant false
     *       P2 evaluates(binary) c to constant true && e and f to constant false
     *       P3 evaluates(binary) d to constant true && e and f to constant false
     *
     * Also, to satisfy the requirement for ~C: (we need to check each contact relation):
     * !(P0 evaluates g to constant true && P1 evaluates h to constant true) && // i.e. (P0 a) belongs to v(g)
     * and (P1 b) belongs to v(h)
     * !(P0 evaluates i to constant true && P1 evaluates j to constant true)
     * and for P2 and P3:
     * !(P2 evaluates g to constant true && P3 evaluates h to constant true) &&
     * !(P2 evaluates i to constant true && P3 evaluates j to constant true)
     *
     * The contact relation is reflexive, so for each point we also need to check also:
     * !(Pi evaluates g to constant true && Pi evaluates h to constant true) && // i.e. (Pi t) point is not in
     * the model evaluation v(g) and v(h) (it's not a common point)
     * !(Pi evaluates i to constant true && Pi evaluates j to constant true)
     *
     */
    auto create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
                const terms_t& zero_terms_F, const variables_mask_t& used_variables, const formula_mgr* mgr)
        -> bool;

    auto get_model_points() const -> const points_t&;
    auto get_variables_evaluations() const -> const variable_id_to_points_t& override;
    auto get_contact_relations() const -> const contacts_t& override;
    auto get_number_of_contacts() const -> size_t;
    auto get_number_of_non_zeros() const -> size_t;
    auto get_number_of_contact_points() const -> size_t;
    auto get_number_of_non_zero_points() const -> size_t;

    void clear();

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
     * Returns true if out_evaluation evaluates all zero terms to false
     */
    auto are_zero_terms_T_satisfied(const terms_t& zero_terms_T,
                                    const variables_evaluations_block& evaluation) const -> bool;
    /*
     * Returns true if for each ~C(a,b) the following is satisfied:
     *  !(out_evaluation evaluates a to true && out_evaluation evaluates b to true), i.e. the reflexivity of
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
                                                   const variables_evaluations_block& evaluation_left,
                                                   const variables_evaluations_block& evaluation_right) const
        -> bool;

    void calculate_the_model_evaluation_of_each_variable();
    // Inserts 1s in the contact relations matrix between 2k and 2k+1 points (where k is less than the number
    // of contacts)
    // because we first create the contact points and then the non-zeros
    void fill_contact_relations();

    friend std::ostream& operator<<(std::ostream& out, const model& m);

    const formula_mgr* mgr_{};
    variables_mask_t used_variables_{};
    size_t number_of_contacts_{};

    points_t points_;

    /*
        Symetric square bit matrix. contacts_[i] gives a bit mask of all points which are in contact with the
       point 'i'.
        For example, let us have a model with 6 points:
            0---1
            2---3
            4
            5

        The bit matrix will be the following:
            \ 012345
            0 010010    // from 0---1
            1 100000    // from 1---0
            2 000100    // from 2---3
            3 001000    // from 3---2
            4 100000    // from 4---0
            5 000000    // no contacts with point 5
    */
    contacts_t contact_relations_;

    /*
        A vector of size @used_variables_, each element is a set of points, represented as a bitset.
        Keeps the variable evaluations, i.e. v(p).

        For example, let us have the following formula: C(a * b, c) & c != 0 & -a != 0
        Then a model could be the following:
            (110) (a * b) 0---1 (c) (001)
            (011) (c)  2
            (000) (-a) 3

        Let assume that 'a' has a variable id 0, 'b' has id 1 and 'c' has id 2.
        The bit matrix will be the following:
            \ 0123     // variable ids (rows) \ model points (columns)
            0 1000     // v(a) = { 0 }, i.e. all points which have evaluation with bit at positon 0 set to 1
            1 1010     // v(b) = { 0, 2 }, ... at position 1 ...
            2 0110     // v(c) = { 1, 2 }, ... at position 2 ...
    */
    variable_id_to_points_t variable_evaluations_; // a vector of bitsets representing the value of v(p)
};
