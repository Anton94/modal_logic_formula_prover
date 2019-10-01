#pragma once

#include "variables_evaluations_block.h"
#include "types.h"

class formula_mgr;
class term;
class formula;

struct model
{
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

        Connectivity axiom:
        a != 0 & a != 1 -> C(a,a*)
        in our case, we evaluate each point's term (i.e., v(a)) and if it's not the whole set of points, we will create a contact relation with an element from v(a*)
    */
    struct point_info
    {
        const term* t;
        variables_evaluations_block evaluation;
        model_points_set_t model_evaluation;

        size_t connectivity_axiom_contact_point_id; // if model_evaluation is not the the whole set, it will be used to make a contact between t and t*, i.e. C(t, t*)
    };
    using points_t = std::vector<point_info>;

    using contacts_t = std::vector<model_points_set_t>;

    // Creates two model points for each contact and one model point for each non zero term, which are paired with a corresponding variable evaluation(binary) for each point
    // The points are created in a sequentional order
    auto construct_model_points(const formulas_t& contacts, const terms_t& non_zero_terms, const variables_mask_t& used_variables, const formula_mgr* mgr_) -> bool;

    // Generates a new model
    auto generate_next_model() -> bool;

    // Generates only a different combination of contacts from the connectivity axiom
    auto generate_next_contact_relations() -> bool;

    // Return true if there is a contact between the v(a) and v(b). Note that v(X) is a set of model points and there is a contact between the two sets iff
    // exist x from v(a), exist y from v(b) and xRy.
    auto is_in_contact(const term* a, const term* b) const -> bool;
    //  Return true if there is a contact between the two given sets of model points
    auto is_in_contact(const model_points_set_t& l, const model_points_set_t& r) const -> bool;
    // Returns true if v(a) is the empty set.
    auto is_empty_set(const term* a) const -> bool;

    auto is_not_in_contact(const term* a, const term* b) const -> bool;
    auto is_not_empty_set(const term* a) const -> bool;

    auto get_model_points() const -> const points_t&;
    auto get_variables_evaluations() const -> const variable_id_to_points_t&;
    auto get_number_of_contacts() const -> size_t;
    auto get_number_of_non_zeros() const->size_t;
    auto get_number_of_contact_points() const-> size_t;
    auto get_number_of_non_zero_points() const-> size_t;
    auto get_contacts() const -> const contacts_t&;

    void clear();

private:
    auto construct_contact_model_points(const formulas_t& contacts) -> bool;
    auto construct_non_zero_model_points(const terms_t& non_zero_terms) -> bool;

    void initialize_contact_relations_from_contacts();
    auto construct_non_zero_term_evaluation(const term* t, variables_evaluations_block& out_evaluation) const -> bool;

    // Generates new evaluation until @t evalautes to constant true with it
    auto generate_next_positive_evaluation(const term* t, variables_evaluations_block& evaluation) const -> bool;

    void calculate_model_evaluation_of_each_variable();
    void calculate_model_evaluation_of_each_point_term();
    void calculate_contact_relations();

    // Generates next contact pair between point's term(t) and it's addition(t*)
    // Returns true if it succeed, otherwise it reset's it and returns false.
    auto generate_next_or_reset_connectivity_axiom_contact(size_t point_id) -> bool;

    friend std::ostream& operator<<(std::ostream& out, const model& m);

    const formula_mgr* mgr_{};
    variables_mask_t used_variables_{};
    size_t number_of_contacts_{};

    points_t points_;

    /*
        Symetric square bit matrix. contacts_[i] gives a bit mask of all points which are in contact with the point 'i'.
        For example, let us have a model with 6 points:
            0---1
            2---3
            4
            5
        from the connectivity axiom: 0---4

        The bit matrix will be the following:
            \ 012345
            0 010010    // from 0---1 & 0---4
            1 100000    // from 1---0
            2 000100    // from 2---3
            3 001000    // from 3---2
            4 100000    // from 4---0
            5 000000    // no contacts with point 5
    */
    contacts_t contact_relations_;
    contacts_t contact_relations_from_contacts_; // the connectivity axiom will change the contact relations when some point's evaluation change, but the rest of the contacts are fixed, so cache them

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
