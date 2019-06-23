#pragma once

#include "term.h"
#include "nlohmann_json\json.hpp"
#include <ostream>

class formula
{
	using json = nlohmann::json;

public:
	formula();
	~formula();

	formula(const formula&) = delete;
	formula& operator=(const formula&) = delete;

	bool build(json& f);

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

	operation_t op_;
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
