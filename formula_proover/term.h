#pragma once

#include "nlohmann_json/json.hpp"
#include <ostream>

class term
{
    using json = nlohmann::json;

public:
    term();
    ~term();
    term(const term&) = delete;
    term& operator=(const term&) = delete;

    bool operator==(const term& rhs) const;

    bool build(json& t /*, which are the variables which will be used for the DNF of the term*/);
    std::size_t get_hash() const;

    friend std::ostream& operator<<(std::ostream& out, const term& t);

private:
    bool is_in_DNF; // TODO: make DNF form, but only when needed
    std::size_t hash_;
    json term_raw;

private:
};

namespace std
{

template <>
struct hash<term>
{
    std::size_t operator()(const term& t) const
    {
        return t.get_hash();
    }
};
}
