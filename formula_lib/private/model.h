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

        Depending on the generation policy, we might want to generate a new evaluation for the point which evaluates the corresponding
        contact term to the constant true (in order that point to be in the model evaluation of that term, i.e. v(a) = { i | i is a contact point }

    */
    struct term_evaluation_t
    {
        const term* t;
        variables_evaluations_block evaluation;
    };
    using model_points_t = std::vector<term_evaluation_t>;

    // Creates two model points for each contact and one model point for each non zero term, which are paired with a corresponding variable evaluation(binray) for each point
    // The points are created in a sequentional order// TODO more info
    auto construct_model_points(const formulas_t& contacts, const terms_t& non_zero_terms, const variables_mask_t& used_variables, const formula_mgr* mgr_) -> bool;

    // Generates a new combination of evaluations for the model points
    auto generate_next_model_points_evaluation_combination() -> bool;

    // Return true if there is a contact between the v(a) and v(b). Note that v(X) is a set of model points and there is a contact between the two sets iff
    // exist x from v(a), exist y from v(b) and xRy.
    auto is_in_contact(const term* a, const term* b) const -> bool;
    // Returns true if v(a) is the empty set.
    auto is_empty_set(const term* a) const -> bool;

    auto is_not_in_contact(const term* a, const term* b) const -> bool;
    auto is_not_empty_set(const term* a) const -> bool;

    auto get_model_points() const -> const model_points_t&;
    auto get_variables_evaluations() const -> const variable_id_to_model_points_t&;
    auto get_number_of_contacts() const -> size_t;

    void clear();

private:
    auto construct_contact_model_points(const formulas_t& contacts) -> bool;
    auto construct_non_zero_model_points(const terms_t& non_zero_terms) -> bool;
    auto construct_non_zero_term_evaluation(const term* t, variables_evaluations_block& out_evaluation) const -> bool;

    // Generates new evaluation until @t evalautes to constant true with it
    auto generate_next_positive_evaluation(const term* t, variables_evaluations_block& evaluation) const -> bool;

    void calculate_the_model_evaluation_of_each_variable();

    friend std::ostream& operator<<(std::ostream& out, const model& m);

    const formula_mgr* mgr_{};
    variables_mask_t used_variables_{};
    size_t number_of_contacts_{};

    model_points_t model_points_;
    variable_id_to_model_points_t variable_ids_model_evaluations_;
};
