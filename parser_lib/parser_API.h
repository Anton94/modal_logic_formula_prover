#pragma once

#include <memory>
#include "ast.h"

/// Parses the @in string and returns an unique pointer to the ast's root node.
/// If the parsing fails returns a nullptr
std::unique_ptr<NFormula> parse_from_input_string(const char* in);
