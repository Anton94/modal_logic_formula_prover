#pragma once

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

private:
    class dnf_node
    {
    protected:
        int x;
        // information for the used tree node.
    };

    auto json_to_tree(json& f);
    auto minimize_tree(dnf_node& root);
    auto tree_to_dnf_term(dnf_node& root);
};
