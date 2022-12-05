#include "../include/imodel.h"

auto imodel::get_variables_evaluations() const -> const variable_id_to_points_t&
{
    return variable_evaluations_;
}

auto imodel::get_contact_relations() const -> const contacts_t&
{
    return contact_relations_;
}

auto imodel::get_model_points() const -> const points_t&
{
    return points_;
}

auto imodel::print(std::ostream& out) const -> std::ostream&
{
    return out;
}

void imodel::clear()
{
    contact_relations_.clear();
    variable_evaluations_.clear();
    variable_names_.clear();
    points_.clear();
}

auto imodel::get_variable_name(variable_id_t variable_id) const -> const std::string&
{
    if(variable_id >= variable_names_.size())
    {
        static const std::string undefined = "UNDEFINED NAME";
        return undefined;
    }

    return variable_names_[variable_id];
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

auto imodel::print(std::ostream& out, const variables_evaluations_block& block) const -> std::ostream&
{
    const auto& evaluations = block.get_evaluations();
    const auto variables = block.get_variables();
    for(size_t i = 0; i < variables.size(); ++i)
    {
        if(variables[i])
        {
            out << get_variable_name(i) << ":" << evaluations.test(i) << " ";
        }
    }

    return out;
}

auto imodel::print(std::ostream& out, const points_t& points) const -> std::ostream&
{
    for(size_t i = 0; i < points.size(); ++i)
    {
        out << i << " : ";
        print(out, points[i]);
        out << "\n";
    }

    return out;
}

auto imodel::print_contacts(std::ostream& out, const contacts_t& contact_relations) const -> std::ostream&
{
    for (size_t a = 0; a < contact_relations.size(); ++a)
    {
        const auto& points_in_contact_with_a = contact_relations[a];

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

    return out;
}

auto imodel::print_evaluations(std::ostream& out, const variable_id_to_points_t& variable_evaluations) const -> std::ostream&
{
    for(size_t i = 0; i < variable_evaluations.size(); ++i)
    {
        const auto& variable_evaluation_bitset = variable_evaluations[i];

        out << "v(" << get_variable_name(i) << ") = {";

        auto model_point_id = variable_evaluation_bitset.find_first();
        auto is_first = true;
        while(model_point_id != variables_evaluations_t::npos)
        {
            out << (is_first ? " " : ", ") << model_point_id;
            is_first = false;
            model_point_id = variable_evaluation_bitset.find_next(model_point_id);
        }
        out << " }\n";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const imodel& m)
{
    out << "Model points: \n";
    m.print(out, m.points_);

    out << "Model contact relations: \n";
    m.print_contacts(out, m.contact_relations_);

    out << "Model evaluation of the variables: \n";
    m.print_evaluations(out, m.variable_evaluations_);

    m.print(out);

    return out;
}
