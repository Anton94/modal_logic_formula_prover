#include "parser_API.h"

#include "parser.h"

thread_local std::unique_ptr<NFormula> parsed_formula;


std::unique_ptr<NFormula> parse_from_input_string(const char* in)
{
    parsed_formula.reset(nullptr);

    yyscan_t scanner;
    yylex_init(&scanner);

    YY_BUFFER_STATE buff = yy_scan_string(in, scanner);
    yyset_lineno(1, scanner);
    yyset_column(0, scanner);

    yyparse(scanner); // 0 is OK 1,2 are errors, might be useful.
    yy_delete_buffer(buff, scanner);
    yylex_destroy(scanner);

    return std::unique_ptr<NFormula>(parsed_formula.release());
}
