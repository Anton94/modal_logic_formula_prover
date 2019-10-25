#pragma once

#include <string>
#include <memory>
#include <ostream>
#include "ast.h"


struct parser_error_info
{
    int line;
    int column;
    std::string msg;

    void printer(const std::string& parsed_formula, std::ostream& out) const;
};

/// Parses the @in string and returns an unique pointer to the ast's root node.
/// If the parsing fails returns a nullptr set's the errors information.
std::unique_ptr<NFormula> parse_from_input_string(const char* in, parser_error_info& info);
