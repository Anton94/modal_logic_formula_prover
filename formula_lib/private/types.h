#pragma once

#include "../include/types.h"
#include "boost/dynamic_bitset.hpp"
#include <vector>

using full_variables_evaluations_t = std::vector<bool>;

using variable_id_t = size_t;

// a bit mask - set bit at position X indicates that the variable with id X participates in the container
using variables_mask_t = boost::dynamic_bitset<>;

using variables_evaluations_t = boost::dynamic_bitset<>;
