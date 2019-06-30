#include "dnf-transformer.h"

auto dnf_transformer::transform_to_dnf_term(json& f)
{
	return dnf_term();
}

auto dnf_transformer::json_to_tree(json& f)
{
	return dnf_node();
}

auto dnf_transformer::minimize_tree(dnf_node& root)
{
	return dnf_node();
}

auto dnf_transformer::tree_to_dnf_term(dnf_node& root)
{
	return dnf_term();
}

