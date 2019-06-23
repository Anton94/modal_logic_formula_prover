#pragma once

#include "nlohmann_json\json.hpp"
#include <ostream>

class term
{
	using json = nlohmann::json;

public:
	term();
	~term();
	term(const term&) = delete;
	term& operator=(const term&) = delete;

	bool build(json& t/*, which are the variables which will be used for the DNF of the term*/);

	friend std::ostream& operator<<(std::ostream& out, const term& t);
private:

	bool is_in_DNF;// TODO: make DNF form, but only when needed

	json term_raw;
private:
};
