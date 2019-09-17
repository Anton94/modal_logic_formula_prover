#pragma once

#include "../include/types.h"
#include "boost/dynamic_bitset.hpp"
#include <vector>

using full_variables_evaluations_t = std::vector<bool>;

using variable_id_t = size_t;

// a bit mask - set bit at position X indicates that the variable with id X participates in the container
using variables_mask_t = boost::dynamic_bitset<>;
using variables_evaluations_t = boost::dynamic_bitset<>;

class formula;
class term;

struct formula_ptr_hasher
{
    auto operator()(const formula* const& f) const -> std::size_t;
};

struct formula_ptr_comparator
{
    auto operator()(const formula* const& lhs, const formula* const& rhs) const -> bool;
};

struct term_ptr_hasher
{
    auto operator()(const term* const& t) const -> std::size_t;
};

struct term_ptr_comparator
{
    auto operator()(const term* const& lhs, const term* const& rhs) const -> bool;
};

using formulas_t = std::unordered_set<const formula*, formula_ptr_hasher, formula_ptr_comparator>;
using terms_t = std::unordered_set<const term*, term_ptr_hasher, term_ptr_comparator>;

using multiterms_t = std::unordered_multiset<const term*, term_ptr_hasher, term_ptr_comparator>;
using multiterm_to_formula_t =
    std::unordered_multimap<const term*, const formula*, term_ptr_hasher, term_ptr_comparator>;

using model_points_set_t = boost::dynamic_bitset<>;
using variable_id_to_model_points_t = std::vector<model_points_set_t>;
using model_points_sets_t = std::vector<model_points_set_t>;

std::ostream& operator<<(std::ostream& out, const formulas_t& formulas);
std::ostream& operator<<(std::ostream& out, const terms_t& terms);
std::ostream& operator<<(std::ostream& out, const multiterms_t& terms);
std::ostream& operator<<(std::ostream& out, const multiterm_to_formula_t& mapping);
