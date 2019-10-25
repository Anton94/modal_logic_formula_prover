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
    input_formula = "C(a,b) | a * b + G * g = 0";
    input_formula = "F | <=(f + g + -x, -h) ";
    input_formula = "~(C(a,0) | a * m + b + -h= 0) | (<=m(mmax,m) | <=(m1,Cm))";
    input_formula = "<=(a + b, c) | C(a + b, c) & C(a, b + c) & C(a + b + c, e + f + g)";
    input_formula = "<=(a + b, c) | <=(a + b + c * -h, e + f * g)";
    input_formula = "C(a+b,1) & C(1,e+g * -h) & C(a+b,0)";
    input_formula = "C(a,b) | <=(E,fafgs) \n C(b,a) | <=(fasfa,e)";

    std::cout << "Will try to parce         : " << input_formula << std::endl;
    parser_error_info error_info;
    const auto formula_ast = parse_from_input_string(input_formula, error_info);

    if(!formula_ast)
    {
        error_info.printer(input_formula, std::cout);
        return -1;
    }

    std::cout << "Parsed formula            : ";
    VPrinter printer(std::cout);
    formula_ast->accept(printer);
    std::cout << std::endl;

    VConvertImplicationEqualityToConjDisj convertor;
    formula_ast->accept(convertor);
    std::cout << "Converted (-> <->)        : ";
    formula_ast->accept(printer);
    std::cout << std::endl;

    VConvertLessEqContactWithEqualTerms convertor_lessEq_contact_with_equal_terms;
    formula_ast->accept(convertor_lessEq_contact_with_equal_terms);
    std::cout << "Converted C(a,a);<=(a,a)  : ";
    formula_ast->accept(printer);
    std::cout << std::endl;

    VSplitDisjInLessEqAndContacts disj_in_contact_splitter;
    formula_ast->accept(disj_in_contact_splitter);
    std::cout << "C(a+b,c)->C(a,c)|C(b,c) ;\n";
    std::cout << "<=(a+b,c)-><=(a,c)&<=(b,c): ";
    formula_ast->accept(printer);
    std::cout << std::endl;

    VConvertLessEqToEqZero eq_zero_convertor;
    formula_ast->accept(eq_zero_convertor);
    std::cout << "Converted (<= =0) formula : ";
    formula_ast->accept(printer);
    std::cout << std::endl;

    VReduceConstants trivial_reducer;
    formula_ast->accept(trivial_reducer);
    std::cout << "Reduced constants         : ";
    formula_ast->accept(printer);
    std::cout << std::endl;

    VConvertContactsWithConstantTerms contacts_with_constant_as_term_convertor;
    formula_ast->accept(contacts_with_constant_as_term_convertor);
    std::cout << "Converted C(a,1)->~(a=0)  : ";
    formula_ast->accept(printer);
    std::cout << std::endl;

    VReduceDoubleNegation double_negation_reducer;
    formula_ast->accept(double_negation_reducer);
    std::cout << "Reduced double negation   : ";
    formula_ast->accept(printer);
    std::cout << std::endl;

    return 0;
}
