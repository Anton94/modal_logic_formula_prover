#include <iostream>
#include <string>
#include <fstream>

#include "cmd_options/cxxopts.hpp"
#include "library.h"

#include <include/emscripten/emscripten.h>

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

inline const char* cstr(const std::string& message)
{
    char * cstr = new char [message.length() + 1]; // + terminating null
    std::strcpy(cstr, message.c_str());
    return cstr; // JS will free it
}

auto get_output_stream() -> std::stringstream&
{
    static std::stringstream output;
    return output;
}

EXTERN EMSCRIPTEN_KEEPALIVE const char* calculate(const char* js_formula, int len)
{
    get_output_stream() = {};

    std::string formula(js_formula, len);
    // formula = "~(x = 0) & ~(y = 0) & ~(<=(x,y)) & ~(<=(y,x)) & <=m(x, x + y) & <=m(x+y, x) & <=m(x2, x3) & (x3 = 0)";

    formula_mgr f;
    info() << "\t" << (f.build(formula) ? "success" : "failed") << "\n";

    info() << "Getting the used variables...";
    const auto variables = f.get_variables();
    info() << "Variables: " << variables;

    measured_model m;
    info() << "Trying to find a measured model!";
    const auto res = f.is_satisfiable(m);
    info() << "The formula is " << (res ? "" : "not ") << "satisfiable.";
    info() << "Measured model:\n" << m;

    auto output = get_output_stream().str();
    get_output_stream() = {};

    return cstr(output);
}

int main(int argc, char* argv[])
{
    /*
     * This is a special app which is used from JS.
     * Exposes the formula_lib functionality.
     * It is Converted to WebAssembly and gives an API which is used by JS.
     *
     */

    set_trace_logger([](std::stringstream&& s) { get_output_stream() << s.rdbuf() << "\n"; });
    set_info_logger([](std::stringstream&& s) { get_output_stream() << s.rdbuf() << "\n"; });
    set_error_logger([](std::stringstream&& s) { get_output_stream() << "ERROR: " << s.rdbuf() << "\n"; });

    return 0;
}
