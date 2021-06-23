#pragma once

#include "types.h"
#include "variables_evaluations_block.h"
#include <ostream>
#include <unordered_set>

class formula_mgr;
class NTerm;

class term
{
public:
    term(const formula_mgr* mgr);
    ~term();
    term(const term&) = delete;
    term& operator=(const term&) = delete;
    term(term&& rhs) noexcept;
    term& operator=(term&& rhs) noexcept;

    auto operator==(const term& rhs) const -> bool;
    auto operator!=(const term& rhs) const -> bool;

    auto build(const NTerm& t) -> bool;

    auto evaluate(const variable_id_to_points_t& variable_evaluations, const size_t points_count) const
        -> model_points_set_t;

    void clear();

    struct evaluation_result
    {
        enum class result_type : char
        {
            none,
            constant_true,
            constant_false,
            term,
        };

        evaluation_result(result_type res_type, term* t);
        evaluation_result(const evaluation_result&) = delete;
        evaluation_result& operator=(const evaluation_result&) = delete;
        evaluation_result(evaluation_result&& rhs) noexcept;
        evaluation_result& operator=(evaluation_result&& rhs) noexcept;
        ~evaluation_result();

        auto release() -> term*;
        auto get() const -> const term*;
        void free();
        auto is_constant() const -> bool;
        auto is_term() const -> bool;
        auto is_constant_true() const -> bool;
        auto is_constant_false() const -> bool;

        result_type type{result_type::none};

    private:
        void move(evaluation_result&& rhs) noexcept;

        term* t_{nullptr};
    };

    auto evaluate(const variables_evaluations_block& evaluation_block,
                  bool skip_subterm_creation = true) const -> evaluation_result;

    enum class operation_type : char
    {
        constant_true,
        constant_false,

        union_,
        intersection,
        complement,
        variable,

        invalid,
    };
    using operation_t = operation_type;

    /// Creates a constant term
    void construct_constant(bool constant_type);

    auto get_operation_type() const -> operation_t;
    auto get_hash() const -> std::size_t;
    auto get_variable() const -> std::string;
    auto get_variables() const -> const variables_mask_t&;

    // The star operation has only left child
    // Constants and variable operations has no childs.
    // UB when trying to get childs of incompatable types,
    // e.g. it is star operation and taking right child
    auto get_left_child() const -> const term*;
    auto get_right_child() const -> const term*;

    void change_formula_mgr(formula_mgr* new_mgr);

    auto is_binary_operaton() const -> bool;
    auto is_constant() const -> bool;

private:
    void move(term&& rhs) noexcept;

    auto construct_complement_operation(const NTerm& t) -> bool;
    auto construct_variable_operation(const NTerm& t) -> bool;
    auto construct_binary_operation(const NTerm& t, operation_t op) -> bool;

    // calculates and sets the hash depending on the @op_ and the child hashes (if any)
    void construct_hash();
    // calculates and sets the used variables mask depending on the @op_ and the child's variables (if any)
    void construct_variables();

    // creats a term internal node with the given childs(if any). allowed operations are star, union and
    // intersection
    // the user must free the created node!
    auto create_internal_node(operation_t op, bool skip_subterm_creation, term* left = nullptr,
                              term* right = nullptr) const -> term*;
    // creats a term variable/leaf node
    // the user must free the created node!
    auto create_variable_node(size_t variable_id, bool skip_subterm_creation) const -> term*;

    void free();

    operation_t op_;
    // A pointer, because the formula_mgr could be moved, so it could change.
    // Used to make a bitmask for the used variables in the term(w.r.t all variables in the whole formula) and to print the term with it's variable names instead of their (optimized) IDs.
    // Yes, we can get rid of it but will have to keep mapping of all used variables ids to their names, etc.
    const formula_mgr* formula_mgr_;

    variables_mask_t variables_; // TODO: alternative is to calculate the used variables each time when we
                                 // need them. would it be better?

    // TODO use variant and unique_ptrs instead of raw pointers
    struct childs
    {
        term* left;
        term* right;
    };
    union {
        childs childs_;
        size_t variable_id_;
    };

    std::size_t hash_;
};

std::ostream& operator<<(std::ostream& out, const term& t);
std::ostream& operator<<(std::ostream& out, const term::evaluation_result& res);

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
