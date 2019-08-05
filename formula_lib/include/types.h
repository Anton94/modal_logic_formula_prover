#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using variables_set_t = std::unordered_set<std::string>;
std::ostream& operator<<(std::ostream& out, const variables_set_t& variables);

using variables_t = std::vector<std::string>;
std::ostream& operator<<(std::ostream& out, const variables_t& variables);

using variable_to_evaluation_map_t = std::vector<std::pair<std::string, bool>>;
std::ostream& operator<<(std::ostream& out, const variable_to_evaluation_map_t& variables);
