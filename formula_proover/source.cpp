#include <iostream>
#include <string>
#include <fstream>

#include "cmd_options/cxxopts.hpp"
#include "nlohmann_json/json.hpp"

#include "formula_mgr.h"
#include "logger.h"
#include "tableau.h"

using json = nlohmann::json;

int main(int argc, char* argv[])
{
    set_trace_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    set_info_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    set_error_logger([](std::stringstream&& s) { std::cout << "ERROR: " << s.rdbuf() << std::endl; });

    error();
    try
    {
        cxxopts::Options options("FormulaProover", "One line description of MyProgram");

        options.add_options()
                ("h,help", "Print help")
                ("f,formula", "Formula in json fomrat", cxxopts::value<std::string>())
                ("i,input_file", "File containing a formula in json fomrat", cxxopts::value<std::string>());

        auto result = options.parse(argc, argv);

        if(result.count("help"))
        {
            info() << options.help();
            return 0;
        }

        json formula_json;
        if(result.count("formula"))
        {
            auto formula_arg = result["formula"].as<std::string>();

            info() << "Parsing forumla: " << formula_arg;
            formula_json = json::parse(formula_arg);
        }
        if(result.count("input_file"))
        {
            auto input_file_path = result["input_file"].as<std::string>();
            info() << "Reading a formula from an input file: " << input_file_path;
            std::ifstream in(input_file_path);

            formula_json = json::parse(in);
        }

        info() << "Parsed into json:\n" << formula_json.dump(4);

        info() << "Building a formula tree with the parsed one...";
        formula_mgr f;
        info() << "\t" << (f.build(formula_json) ? "success" : "failed") << " " << f;

        info() << "Getting the used variables...";
        const auto variables = f.get_variables();
        info() << "Variables: " << variables;

        tableau t;
        info() << "The formula is " << (t.is_satisfiable(f) ? "" : "not ") << "satisfiable.";
    }
    catch(const cxxopts::OptionException& e)
    {
        error() << "arg error: " << e.what();
    }

    return 0;
}
