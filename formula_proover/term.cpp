#include "term.h"

#include <cassert>

term::term()
    : is_in_DNF(false)
    , hash_(0ul)
{
}

term::~term()
{
}

bool term::operator==(const term& rhs) const
{
    return hash_ == rhs.hash_ && term_raw == rhs.term_raw;
}

bool term::build(json& t)
{
    if(!t.contains("value"))
    {
        return false;
    }

    term_raw = t["value"];
    hash_ = std::hash<json>()(term_raw);

    return true;
}

std::size_t term::get_hash() const
{
    return hash_;
}

std::ostream& operator<<(std::ostream& out, const term& t)
{
    out << t.term_raw.get<std::string>(); // TODO: support complex terms
    return out;
}
