#pragma once

#include "term.h"
#include "nlohmann_json/json.hpp"
#include <ostream>

class formula
{
	using json = nlohmann::json;

public:
	formula();
	~formula();

	formula(const formula&) = delete;
	formula& operator=(const formula&) = delete;

	formula(formula&&) noexcept = default;
	formula& operator=(formula&&) noexcept = default;

	bool operator==(const formula& rhs) const;

	bool build(json& f);
	std::size_t get_hash() const;

	friend std::ostream& operator<<(std::ostream& out, const formula& f);
private:

	enum class operation_type : char
	{
		conjunction,
		disjunction,
		negation,
		le,
		c,

		invalid,
	};
	using operation_t = operation_type;

	static std::string& operation_to_symbol(operation_t op);
	bool is_term_operation() const;
	bool is_formula_operation() const;

	bool create_terms(json& f);
	bool create_formulas(json& f);

	operation_t op_;
	std::size_t hash_;
	union
	{
		struct
		{
			formula* left_f_;
			formula* right_f_;
		};
		struct
		{
			term* left_t_;
			term* right_t_;
		};
	};
};

namespace std
{

template<>
struct hash<formula>
{
	std::size_t operator()(const formula& f) const
	{
		return f.get_hash();
	}
};

}
