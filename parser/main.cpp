#include "parser.h"

int main(int, char**)
{
    const auto input_formula = "~((C(a, b) & F) & <=m((a * b), (c + d)))";
    return parse_from_input_string(input_formula);
}
