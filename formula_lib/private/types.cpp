#include "types.h"

std::ostream& operator<<(std::ostream& out, const variables_set_t& variables)
{
    for(auto& variable : variables)
    {
        out << variable << " ";
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const variables_t& variables)
{
    for (const auto variable : variables)
    {
        out << variable << " ";
    }

    return out;
}
