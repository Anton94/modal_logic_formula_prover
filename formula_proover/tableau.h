#pragma once

#include "formula.h"

#include <unordered_set>

class tableau
{
public:
	tableau(const tableau&) = delete;
	tableau& operator=(const tableau&) = delete;
	tableau(tableau&&) = default;
	tableau& operator=(tableau&&) noexcept = default;

	auto proof(const formula& f) -> bool;

private:
	using formulas_t = std::unordered_set<const formula*>;
	formulas_t T_formulas_;
	formulas_t F_formulas_;
};
