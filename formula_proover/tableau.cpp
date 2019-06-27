#include "tableau.h"

auto tableau::proof(const formula& f) -> bool
{
    // if the negation of @f is not satisfiable, then @f is tautology
    F_formulas_.insert(&f);

    // TODO: do the magic

    return true;
}
