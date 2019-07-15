#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using variables_set_t = std::unordered_set<std::string>;
std::ostream& operator<<(std::ostream& out, const variables_set_t& variables);
