#pragma once

#include <iostream>
#include <iomanip>
#include "nlohmann_json/json.hpp"
#include <unordered_set>

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
    auto transform_to_dnf_term(json& f);

// TODO REMOVE THIS PUBLIC
public:
	class dnf_node
    {
	public:
		dnf_node(std::string x, dnf_node* left = NULL, dnf_node* right = NULL) {
			x_ = x;
			left_ = left;
			right_ = right;
		}

		~dnf_node() {
			if (left_ != NULL)
				delete left_;
			if (right_ != NULL)
				delete right_;
		}

		void print(dnf_node& node, int offset) {
			std::cout << std::setw(offset) << node.x_ << std::endl;
			if (node.left_) {
				print(*node.left_, offset + 5);
			}
			if (node.right_) {
				print(*node.right_, offset + 5);
			}
		}
    public:
        std::string x_;
		dnf_node* left_;
		dnf_node* right_;
        // information for the used tree node.
    };

    dnf_node& json_to_tree(json& f);
	dnf_node& negative_normal_form(dnf_node& root);
    auto minimize_tree(dnf_node& root);
    auto tree_to_dnf_term(dnf_node& root);
};
