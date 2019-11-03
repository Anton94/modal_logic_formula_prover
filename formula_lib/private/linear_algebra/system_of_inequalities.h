#pragma once

#include <boost/dynamic_bitset.hpp>
#include <memory>

class system_of_inequalities_impl;

class system_of_inequalities
{
    /*
     * Very special system of inequalities of the following type:
     *
     * | SUM Xi <= SUM Xj    , where Xi is a variable and belongs to some set I and Xj is a variable and belongs to another set J(may be the same, doesn't matter)
     * | SUM Xk <= SUM Xm    , where Xk is a variable and belongs to some set K and Xm is a variable and belongs to another set M
     * | ...
     * | SUM Xl <= SUM Xp    , where Xl is a variable and belongs to some set L and Xp is a variable and belongs to another set P
     *
     * For conviniency, the system_of_inequalities's ctor set's the number of variables which will be used in the system.
     * Then, the adding of new inequality can be done via a bitset which indicates which variables to be summed, e.g.
     *    For SUM Xi <= SUM Xj,
     *    a lhs bitset (of size @number_of_variables) for the set I and
     *    a rhs bitset(again, of size @number_of_variables) for the set J.
     *    (1 at position 'n' in the bitset means that the n-th variable is in this bitset and will be summed)
     *
     * Note that the operation in each row can be <= or >.
     * Searching for a strictly positive solution! (i.e. Xi > 0 for each variable)
     */
public:
    system_of_inequalities(size_t number_of_variables);
    ~system_of_inequalities();

    system_of_inequalities(system_of_inequalities&& rhs) noexcept;
    system_of_inequalities& operator=(system_of_inequalities&& rhs) noexcept;

    using variables_set = boost::dynamic_bitset<>;

    enum class inequality_operation
    {
        LE, // less or equal
        G, // greater
    };

    /*
     * Returns true if the system is solvable and the added inequality does not makes the system unsolvable.
     */
    auto add_constraint(const variables_set& lhs, const variables_set& rhs, inequality_operation op) -> bool;

    /*
     * Returns true if the system is still solvable.
     */
    auto is_solvable() const -> bool;

    /*
     * If the system is solvable, returns a vector of values, which satisfy the system.
     * Element at position 'i' is the value of the i-th variable.
     * If the system is not solvable, returns an empty vector.
     */
    auto get_variables_values() const -> std::vector<double>;

    void clear();

private:
    // pimpl idiom
    std::unique_ptr<system_of_inequalities_impl> impl_;
};
