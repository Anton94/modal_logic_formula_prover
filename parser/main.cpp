#include <iostream>

#include "parser_API.h"
#include "visitor.h"

int main(int, char**)
{
    const char* input_formula = "F";
    input_formula = "~((C(a, b) & F) & <=m((a * b), (c + d)))";

    input_formula = "C(a,b) | C(e * x * -y + z,f * g + -e) & C(a,b)";
    input_formula = "C(a,b) | C(e * x * -(y + z) + t, f * g + -e) & C(a,b)";
    input_formula = "C(a * 1, b + -0 * 1) | C(1 * x * -(y + z) + t, f * g + -0) & C(a,b) | ~~F";
    input_formula = "((F -> C(0,0)) | C(x,x)) & (C(a,b) -> C(b,a)) & (<=(a * c, b * d + f) <-> C(j + -k, k + o * v))";

    std::cout << "Will try to parce: " << input_formula << std::endl;
    const auto formula_ast = parse_from_input_string(input_formula);

    if(!formula_ast)
    {
        return -1;
    }

    std::cout << "Parsed formula     : ";
    VPrinter printer(std::cout);
    formula_ast->accept(printer);
    std::cout << std::endl;

    VConvertImplicationEqualityToConjDisj convertor;
    formula_ast->accept(convertor);
    std::cout << "Converted formula  : ";
    formula_ast->accept(printer);
    std::cout << std::endl;

    VReduceTrivialAndOrNegOperations reducer;
    formula_ast->accept(reducer);

    std::cout << "Reduced formula    : ";
    formula_ast->accept(printer);
    std::cout << std::endl;

    return 0;
}
