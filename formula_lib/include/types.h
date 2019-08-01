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

using human_readable_variables_evaluations_t = std::vector<std::pair<std::string, bool>>;
std::ostream& operator<<(std::ostream& out, const human_readable_variables_evaluations_t& variables);
