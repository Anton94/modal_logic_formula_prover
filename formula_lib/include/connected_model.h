#pragma once

#include "imodel.h"
#include "types.h"
#include "../private/variables_evaluations_block.h"

class formula_mgr;
class term;
class formula;

struct connected_model : public imodel
{
    using points_t = std::vector<variables_evaluations_block>;
    /*
     * Let us have the following formula: C(Ai, Bi) & ~C(Ej, Fj) & Dk!=0 & Gl=0 (i,j,k,l can iterate over some ranges)
     * Let put some notation: N - number of variables, M - number of atomic formulas
     * I - number of contacts, J - number of ~C, K - number of != 0 and L - number of =0
     * 1) Creates all possible unique model points(the model point is just a variable evaluation Pi).
     *   We have N variables, this are 2^N unique points(evaluations).
     *   We skip the once which interfere with the zero terms, i.e. we remove all Pi which evaluates some term Gl to constant true.
     *   We also skip the once which evaluates Ej and Fj to 1 (because of the point's reflexivity it will interfere with ~C atomic formulas)
     *
     *   NOTE that if Pi evaluates the term 't' to constant true then the model evaluation v(t) contains the
     *   point Pi (and vice versa).
     *
     * So far, the model satisfies all =0 atomic formulas and 'half' of the ~C (only the reflexivity of the points)
     *
     * 2) We check if there is some Pi which evaluates != atomics and contact's terms to true, i.e.
     *   there is Px which evalutes Ai to 1, Py - Bi to 1 and Pz - Dk to 1 (for all i and k). (Px,Py,Pz can be same or different)
     *   If there are such evaluations then there is still a chance to find a connected model, otherwise there is no model, not even non-connected one.
     *
     * So far, we have need O(2^N * N) bits of memory to hold all points and their evaluations and O(2^N * M) time.
     *
     * 3) We build connectivity bit matrix 2^N X 2^N and set all cells to 1 (i.e. we make each point connected to all points)
     *   O(2^N * 2^N) === O(2^2N) bits
     *   We remove all connections which interfere with ~C atomic formulas(not that the reflexivity is already satisfied):
     *     for each a, b < 2^N (each pair of points) and for each j : Pa[Ej]=1 & Pb[Fj]=1 OR Pa[Fj]=1 & Pb[Ej]=1
     *     then we remove the connection between point 'a' and 'b'
     *   O(2^N * 2^N * J) time complexity
     *
     * 4) We check if C(Ai, Bi) is still satisfied, i.e. we have not removed all connections between v(Ai) and v(Bi)
     *   for each a, b < 2^N (each pair of points IN CONTACT) and for each i : Pa[Ai]=1 & Pb[Bi]=1 OR Pa[Bi]=1 & Pb[Ai]=1
     *   O(2^N * 2^N * I) time complexity
     *
     * 5) Now we have a 'maximal' unique satisfiable model, which is a sort of graph. We need to find all connected subgraphs in it and if some of them
     *   is also a satisfiable model then this will be our connected model. Finding all connected subgraphs is a simple tax - O(2^N + 2^N * 2^N), i.e.
     *   O(V + E) (vertexes + edges).
     *  Checking if a subset of points satisfies the formula can be done in a similar fashion(as the one above)
     *  Let's Y is the number of points in the subset, then at most O(Y * Y * M) time complexity.
     *  Note that we have to do it for each connected subgraph and the subgraphs are not overlapping!
     *  Upper bound for Y is 2^N, so we can safelty put an upper bound and on the time cimplexity: O(2^N * 2^N * M)
     *  bacause e.g. x * x * M + y * y * M <<< (x+y)*(x+y)*M
     *
     *
     * Overall memory complexity: O(2^N * 2^N) bits
     * Overall time complexity: O(2^N * 2^N * M)
     *
     * TODO: update the comment with the changes in the implementation
    */

    auto create(const formulas_t& contacts_T, const formulas_t& contacts_F, const terms_t& zero_terms_T,
                const terms_t& zero_terms_F, const variables_mask_t& used_variables, const formula_mgr* mgr)
        -> bool override;

    auto get_model_points() const -> const points_t&;

    auto print(std::ostream& out) const -> std::ostream& override;

    void clear() override;

    ~connected_model() override = default;

private:
    /*
     * Constructs all model points(evaluations) which are not breaking the a=0 and ~C(a,b) atomic formulas,
     * i.e. all Pi (a variable evaluation for all N variables) which are evaluating
     * all terms 'a' to false and the following is true: !(Pi[a]=1 && Pi[b]=1) === Pi[a]=0 || Pi[b]=0
     */
    void construct_all_valid_unique_points(const formulas_t& contacts_F, const terms_t& zero_terms_T);
    /*
     * Returns true if evaluation evaluates all zero terms to false
     */
    auto are_zero_terms_T_satisfied(const terms_t& zero_terms_T,
                                    const variables_evaluations_block& evaluation) const -> bool;
    /*
     * Returns true if for each ~C(a,b) the following is satisfied:
     *  !(evaluation evaluates a to true && evaluation evaluates b to true)
     */
    auto is_contacts_F_rule_satisfied_only_reflexivity(const formulas_t& contacts_F,
                                                       const variables_evaluations_block& evaluation) const -> bool;

    /*
     * Returns true if all non-zero term's model evaluations are not empty.
     */
    auto is_zero_terms_F_rule_satisfied(const terms_t& zero_terms_F) const -> bool;

    /*
     * Returns true if all contact term's model evaluations are not empty.
     */
    auto is_contacts_T_existence_rule_satisfied(const formulas_t& contacts_T) const -> bool;

    /*
     * Returns true if for each C(a,b) there is a contact in @contact_relations_ between v(a) and v(b).
     */
    auto is_contacts_T_rule_satisfied(const formulas_t& contacts_T) const -> bool;

    /*
     * For given C(a,b) returns true if there is a contact between v(a) and v(b).
     */
    auto is_contact_satisfied(const formula* c) const -> bool;

    /*
     * Builds N x N bit matrix with contact relations between all points,
     * which later reduces in order to satisfy all ~C atomic formulas.
     * Returns true if the produced contact relations satisfy all contact atomic formulas.
     */
    auto build_contact_relations_matrix(const formulas_t& contacts_T, const formulas_t& contacts_F) -> bool;

    /*
     * Returns a vector of set of points, each of which is a connected component (w.r.t. @contact_relations_)
     */
    auto get_connected_components() const -> std::vector<model_points_set_t>;
    auto get_connected_component(size_t root_point_id, model_points_set_t& not_visited_points) const -> model_points_set_t;

    void reduce_variable_evaluations_to_subset_of_points(const model_points_set_t& points_subset);
    void reduce_model_to_subset_of_points(const model_points_set_t& points_subset);

    void calculate_the_model_evaluation_of_each_variable();

    variables_mask_t used_variables_{};

    points_t points_;
};
