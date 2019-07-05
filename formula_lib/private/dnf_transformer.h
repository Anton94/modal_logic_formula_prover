#pragma once

#include "nlohmann_json/json.hpp"
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <string>

class dnf_transformer
{
    using json = nlohmann::json;
    using dnf_term = std::unordered_set<std::vector<bool>>;

public:
    /// Returns a set of k-monoms by a given json content
    /// 1 step: Create a normal tree from the json where
    ///		the nodes are operations or variables.
    /// 2 step: minimize the tree by combining the variables
    ///		which are binded by conjunction operation together
    ///		this produces k-monoms and reduces the operation nodes.
    /// 3 step: convert the tree of only disjunctions to a set of
    ///		k-monoms.

    // TODO REMOVE THIS PUBLIC
public:
    class dnf_node
    {
    public:
        dnf_node(const std::string& x, dnf_node* left = nullptr, dnf_node* right = nullptr);
        ~dnf_node();
        dnf_node(const dnf_node&) = delete;
        dnf_node& operator=(const dnf_node&) = delete;
        dnf_node(dnf_node&&) noexcept = default;
        dnf_node& operator=(dnf_node&&) noexcept = default;

        void print(dnf_node& node, int offset)
        {
            std::cout << std::setw(offset) << node.x_ << std::endl;
            if(node.left_)
            {
                print(*node.left_, offset + 5);
            }
            if(node.right_)
            {
                print(*node.right_, offset + 5);
            }
        }

    public:
        std::string x_;
        dnf_node* left_;
        dnf_node* right_;
        // information for the used tree node.
    };

    auto transform_to_dnf_term(json& f) const -> dnf_term;

    dnf_node& json_to_tree(json& f);
    dnf_node& negative_normal_form(dnf_node& root);
    // TODO refactor
//    auto minimize_tree(dnf_node& root);
//    auto tree_to_dnf_term(dnf_node& root);
};
