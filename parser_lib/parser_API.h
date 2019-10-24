#pragma once

#include <memory>
#include "ast.h"

std::unique_ptr<NFormula> parse_from_input_string(const char* in);
