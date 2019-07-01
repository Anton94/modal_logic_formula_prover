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

auto term::operator==(const term& rhs) const -> bool
{
    return hash_ == rhs.hash_ && term_raw == rhs.term_raw;
}

auto term::build(json& t) -> bool
{
    if(!t.contains("value"))
    {
        return false;
    }

    term_raw = t["value"];
    hash_ = std::hash<json>()(term_raw);

    return true;
}

auto term::get_hash() const -> std::size_t
{
    return hash_;
}

std::ostream& operator<<(std::ostream& out, const term& t)
{
    out << t.term_raw.get<std::string>(); // TODO: support complex terms
    return out;
}
