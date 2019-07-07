#pragma once

#include <unordered_set>
#include <string>
#include <ostream>

using variables_t = std::unordered_set<std::string>;
std::ostream& operator<<(std::ostream& out, const variables_t& variables);
