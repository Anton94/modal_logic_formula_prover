#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <ostream>

using variables_t = std::unordered_set<std::string>;
std::ostream& operator<<(std::ostream& out, const variables_t& variables);

using variable_evaluations_t = std::unordered_map<std::string, bool>;
std::ostream& operator<<(std::ostream& out, const variable_evaluations_t& variables);
