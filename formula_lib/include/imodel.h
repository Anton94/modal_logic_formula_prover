#pragma once

#include "types.h"
#include "../private/types.h"
#include "../private/variables_evaluations_block.h" // TODO remove the private dependency in the header files
#include <vector>

class formula_mgr;

class imodel
{
public:
    using points_t = std::vector<variables_evaluations_block>;

    virtual auto create(const formulas_t& contacts_T, const formulas_t& contacts_F,
                        const terms_t& zero_terms_T,  const terms_t& zero_terms_F,
                        const formulas_t& measured_less_eq_T, const formulas_t& measured_less_eq_F,
                        const variables_mask_t& used_variables, const variables_t& variable_names) -> bool = 0;

    virtual auto get_variables_evaluations() const -> const variable_id_to_points_t&;
    virtual auto get_contact_relations() const -> const contacts_t&;

    virtual auto get_model_points() const -> const points_t&;

    virtual auto print(std::ostream& out) const -> std::ostream&;

    virtual void clear();
    virtual ~imodel() = default;

    friend std::ostream& operator<<(std::ostream& out, const imodel& m);

protected:
    auto get_variable_name(variable_id_t variable_id) const -> const std::string&;

    auto print(std::ostream& out, const variables_evaluations_block& block) const -> std::ostream&;
    auto print(std::ostream& out, const points_t& points) const -> std::ostream&;
    auto print_contacts(std::ostream& out, const contacts_t& contact_relations) const -> std::ostream&;
    auto print_evaluations(std::ostream& out, const variable_id_to_points_t& variable_evaluations) const -> std::ostream&;

    // Useful for models which have their first 2*@number_of_contacts points in contact (point 2k is in contact with point (2k+1))
    // Inserts 1s in the contact relations matrix between points 2k and 2k+1 (for each k in range [0, @number_of_contacts))
    // Inserts 1s in the contact relations matrix between each point and itself (reflexivity).
    // TODO(toni): move to utils!
    void create_contact_relations_first_2k_in_contact(size_t number_of_points, size_t number_of_contacts);

    /*
        Symetric square bit matrix. contacts_[i] gives a bit mask of all points which are in contact with the point 'i'.
        For example, let us have a model with 6 points:
            0---1  // contact between 0 and 1
            2---3  // contact between 2 and 3
            4      // point from !=0 term
            5      // point from !=0 term

        The bit matrix will be the following:
            \ 012345
            0 110000    // from 0---1 and reflexivity (0---0)
            1 110000    // from 1---0 and reflexivity (1---1)
            2 001100    // from 2---3 and reflexivity (2---2)
            3 001100    // from 3---2 and reflexivity (3---3)
            4 000010    // just reflexivity (4---4)
            5 000001    // just reflexivity (5---5)
    */
    contacts_t contact_relations_;

    /*
        A vector of size the number of variables in the formula, each element is a set of points, represented as a bitset.
        Keeps the variable evaluations, i.e. v(p).

        Model evaluation 'v' which for a given term returns a subset of the model points.
        :
         - v(p) = { i | (i) (xxx...x)[p] == 1, i.e. in point 'i' the evaluation of the variable 'p' is 1 },
        where 'p' is a variable(for optimzations it's an id of the string representation of that variable)
         - v(a * b) = v(a) & v(b)
         - v(a + b) = v(a) | v(b)
         - v(-a) = ~v(a)

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

    variables_t variable_names_;

    // The modal points.
    points_t points_;
};
