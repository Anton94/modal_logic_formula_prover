#pragma once

#include "nlohmann_json/json.hpp"
#include <ostream>
#include <unordered_set>

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

    auto build(json& t) -> bool;
    auto get_hash() const -> std::size_t;

    friend std::ostream& operator<<(std::ostream& out, const term& t);

private:
    enum class operation_type : char
    {
        plus,  // union
        minus, // intersection
        star,
        literal,

        invalid,
    };
    using operation_t = operation_type;

    bool is_in_DNF_; // TODO: make DNF form, but only when needed
    operation_t op_;
    term* left_;
    term* right_;
    std::string variable_; // TODO: hide it, or move the variables and keep just references
    std::size_t hash_;

//    std::unordered_set<std::vector<bool>>* dnf_kmonoms_;
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
