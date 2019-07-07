#pragma once

#include "formula.h"
#include "types.h"

namespace evaluator
{

auto has_satisfiable_evaluation(const formula& f) -> bool;

}
