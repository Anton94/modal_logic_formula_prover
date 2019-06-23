#include "term.h"

#include <cassert>

term::term()
	: is_in_DNF(false)
{
}

term::~term()
{
}

bool term::build(json& t)
{
	if (!t.contains("value"))
	{
		return false;
	}

	term_raw = t["value"];

	return true;
}

std::ostream& operator<<(std::ostream& out, const term& t)
{
	out << t.term_raw.get<std::string>(); // TODO: support complex terms
	return out;
}
