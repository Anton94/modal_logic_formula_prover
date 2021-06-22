#include "../include/imodel.h"
#include "formula_mgr.h"

auto imodel::get_variables_evaluations() const -> const variable_id_to_points_t&
{
    return variable_evaluations_;
}

auto imodel::get_contact_relations() const -> const contacts_t&
{
    return contact_relations_;
}

void imodel::clear()
{
    mgr_ = nullptr;
    contact_relations_.clear();
    variable_evaluations_.clear();
}

void imodel::create_contact_relations_first_2k_in_contact(size_t number_of_points, size_t number_of_contacts)
{
    contact_relations_.clear();
    contact_relations_.resize(number_of_points, model_points_set_t(number_of_points)); // Fill NxN matrix with 0s
    for(size_t k = 0; k < number_of_contacts; ++k)
    {
        const auto a = 2 * k;
        const auto b = a + 1;
        contact_relations_[a].set(b); // Sets the b-th bit to 1.
        contact_relations_[b].set(a);
    }

    // Add also the reflexivity
    for(size_t i = 0; i < number_of_points; ++i)
    {
        contact_relations_[i].set(i);
    }
}

std::ostream& operator<<(std::ostream& out, const imodel& m)
{
    m.print(out);

    out << "Model contact relations: \n";
    for (size_t a = 0; a < m.contact_relations_.size(); ++a)
    {
        const auto& points_in_contact_with_a = m.contact_relations_[a];

        auto point_in_contact_with_a = points_in_contact_with_a.find_first();
        // note that every point should have at least one contact(itself)
        out << a << " is in contact with: ";
        while (point_in_contact_with_a != model_points_set_t::npos)
        {
            out << point_in_contact_with_a << " ";
            point_in_contact_with_a = points_in_contact_with_a.find_next(point_in_contact_with_a);
        }
        out << "\n";
    }

    out << "Model evaluation of the variables: \n";
    for(size_t i = 0; i < m.variable_evaluations_.size(); ++i)
    {
        const auto& variable_evaluation_bitset = m.variable_evaluations_[i];

        out << "v(" << m.mgr_->get_variable(i) << ") = {";

        auto model_point_id = variable_evaluation_bitset.find_first();
        auto is_first = true;
        while(model_point_id != variables_evaluations_t::npos)
        {
            out << (is_first ? " " : ", ") << std::to_string(model_point_id);
            is_first = false;
            model_point_id = variable_evaluation_bitset.find_next(model_point_id);
        }
        out << " }\n";
    }

    return out;
}
