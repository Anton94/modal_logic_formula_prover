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

using variable_to_sets_evaluation_map_t = std::vector<std::pair<std::string, std::unordered_set<int>>>;
std::ostream& operator<<(std::ostream& out, const variable_to_sets_evaluation_map_t& variables);

using variable_to_bits_evaluation_map_t = std::vector<std::pair<std::string, std::vector<bool>>>;
std::ostream& operator<<(std::ostream& out, const variable_to_bits_evaluation_map_t& variables);

using variable_evaluation_set = std::unordered_set<int>;
std::ostream& operator<<(std::ostream& out, const variable_evaluation_set& variables);

struct pair_hash
{
    size_t operator()(const std::pair<int, int>& cords) const
    {
        return cords.first * 100 + cords.second;
    }
};

using relations_t = std::unordered_set<std::pair<int, int>, pair_hash>;
std::ostream& operator<<(std::ostream& out, const relations_t& relations);
