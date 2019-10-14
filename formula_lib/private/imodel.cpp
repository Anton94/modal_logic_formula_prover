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

std::ostream& operator<<(std::ostream& out, const imodel& m)
{
    m.print(out);

    // TODO: contact connections print

    out << "Model evaluation of the variables";
    for(size_t i = 0; i < m.variable_evaluations_.size(); ++i)
    {
        const auto& variable_evaluation_bitset = m.variable_evaluations_[i];

        out << "v(" << m.mgr_->get_variable(i) << ") = { ";

        auto model_point_id = variable_evaluation_bitset.find_first();
        while(model_point_id != variables_evaluations_t::npos)
        {
            out << std::to_string(model_point_id) << ", ";
            model_point_id = variable_evaluation_bitset.find_next(model_point_id);
        }
        out << "}";
    }

    return out;
}
