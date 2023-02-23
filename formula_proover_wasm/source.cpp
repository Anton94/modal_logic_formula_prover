#include <iostream>
#include <string>
#include <fstream>

#include "cmd_options/cxxopts.hpp"
#include "library.h"

int main(int argc, char* argv[])
{
    /*
     * This is a special app which is used from JS.
     * Exposes the formula_lib functionality.
     * It is Converted to WebAssembly and gives an API which is used by JS.
     *
     */


    set_trace_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    set_info_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    set_error_logger([](std::stringstream&& s) { std::cout << "ERROR: " << s.rdbuf() << std::endl; });

    try
    {
        cxxopts::Options options("FormulaProoverWasm", "App designed to be used by JS as wasm.");

        options.add_options()
                ("h,help", "Print help")
                ("f,formula", "Formula in string fomrat", cxxopts::value<std::string>());

        auto result = options.parse(argc, argv);

        if(result.count("help"))
        {
            info() << options.help();
            return 0;
        }

        std::string formula = "~(x = 0) & ~(y = 0) & ~(<=(x,y)) & ~(<=(y,x)) & <=m(x, x + y) & <=m(x+y, x) & <=m(x2, x3) & (x3 = 0)";
        if(result.count("formula"))
        {
            auto formula_arg = result["formula"].as<std::string>();

            info() << "Read formula: " << formula_arg << " from command line argument.";
        }

        formula_mgr f;
        info() << "\t" << (f.build(formula) ? "success" : "failed") << "\n";

        info() << "Getting the used variables...";
        const auto variables = f.get_variables();
        info() << "Variables: " << variables;

        measured_model m;
        const auto res = f.is_satisfiable(m);
        info() << "The formula is " << (res ? "" : "not ") << "satisfiable.";
        info() << "Measured model:\n" << m;
    }
    catch(const cxxopts::OptionException& e)
    {
        error() << "arg error: " << e.what();
    }

    return 0;
}
