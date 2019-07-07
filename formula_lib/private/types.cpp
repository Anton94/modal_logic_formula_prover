#include "types.h"

std::ostream& operator<<(std::ostream& out, const variables_t& variables)
{
    for (auto& variable : variables)
    {
        out << variable << " ";
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const variable_evaluations_t& variables)
{
    for (auto& variable : variables)
    {
        out << variable.first << " -> " << variable.second << "; ";
    }
    return out;
}
