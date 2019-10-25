#include "parser_API.h"
#include "parser.h"

#include <cassert>

thread_local std::unique_ptr<NFormula> parsed_formula;

thread_local std::function<void(int /*line*/, int /*column*/, const char* /*msg*/)>
    on_error; // TODO: enhacne with a structure

std::unique_ptr<NFormula> parse_from_input_string(const char* in, parser_error_info& info)
{
    parsed_formula.reset(nullptr);

    yyscan_t scanner;
    yylex_init(&scanner);

    YY_BUFFER_STATE buff = yy_scan_string(in, scanner);
    yyset_lineno(1, scanner);
    yyset_column(0, scanner);

    on_error = [&](int line, int column, const char* msg) {
        info.line = line;
        info.column = column;
        info.msg = msg;
    };

    yyparse(scanner); // 0 is OK 1,2 are errors, might be useful.
    yy_delete_buffer(buff, scanner);
    yylex_destroy(scanner);

    on_error = {};
    return std::unique_ptr<NFormula>(parsed_formula.release());
}

namespace
{

bool find_nth(const std::string& s, int n, char c, size_t& pos)
{
    if(n <= 0)
    {
        pos = 0;
        return true;
    }

    while(n > 0)
    {
        pos = s.find(c, pos);
        if(pos == std::string::npos)
        {
            return false;
        }
        ++pos;
        --n;
    }
    return true;
}
}

void parser_error_info::printer(const std::string& parsed_formula, std::ostream& out) const
{
    assert(line > 0 && column >= 0);

    out << "Parsing error: [" << line << ":" << column << "] " << msg << "\n"
        << "While parsing:\n" << parsed_formula << "\n";

    size_t error_line_begin = 0;
    if(find_nth(parsed_formula, line - 1 /*- the desired line ending*/, '\n', error_line_begin))
    {
        const auto error_line_end = parsed_formula.find('\n', error_line_begin);
        const auto error_line_length =
            error_line_end == std::string::npos ? std::string::npos : (error_line_end - error_line_begin);
        const auto error_line = std::string(parsed_formula, error_line_begin, error_line_length);

        out << "More precisely:\n" << error_line << "\n";
        if(column > 1)
        {
            out << std::string(static_cast<unsigned>(column) - 1u, ' ');
        }
        out << "^: " << msg << "\n";
    }
}
