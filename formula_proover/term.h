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

    term(term&&) noexcept = default;
    term& operator=(term&&) noexcept = default;

    auto operator==(const term& rhs) const -> bool;

    auto build(json& t /*, which are the variables which will be used for the DNF of the term*/) -> bool;
    auto get_hash() const ->std::size_t;

    friend std::ostream& operator<<(std::ostream& out, const term& t);

private:
    bool is_in_DNF; // TODO: make DNF form, but only when needed
    std::size_t hash_;
    json term_raw;
};

namespace std
{

template <>
struct hash<term>
{
    auto operator()(const term& t) const -> std::size_t
    {
        return t.get_hash();
    }
};

}
