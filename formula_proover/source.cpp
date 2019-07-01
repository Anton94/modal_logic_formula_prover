#include <iostream>
#include <string>

#include "cmd_options/cxxopts.hpp"
#include "nlohmann_json/json.hpp"

#include "formula.h"
#include "tableau.h"
#include "logger.h"

using json = nlohmann::json;

int main(int argc, char* argv[])
{
    set_trace_logger([](const std::string& s) { std::cout << s << std::endl;});
    set_info_logger([](const std::string& s) { std::cout << s << std::endl;});
    set_error_logger([](const std::string& s) { std::cerr << s << std::endl;});

    try
    {
        cxxopts::Options options("FormulaProover", "One line description of MyProgram");

        options.add_options()("h,help", "Print help")("f,formula", "Formula in json fomrat",
                                                      cxxopts::value<std::string>());

        auto result = options.parse(argc, argv);

        if(result.count("help"))
        {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if(result.count("formula"))
        {
            auto formula_arg = result["formula"].as<std::string>();

            info() << "Parsing forumla: " << formula_arg;
            json formula_json = json::parse(formula_arg);
            info() << "Parsed into json: \n" << formula_json.dump(4);

            info() << "Building a formula tree with the parsed one...";
            formula f;
            info() << "\t" << (f.build(formula_json) ? "success" : "failed") << " " << f;

            tableau t;
            info() << "The formula is " << (t.is_tautology(f) ? "" : "not ") << "a tautology.";
        }
    }
    catch(const cxxopts::OptionException& e)
    {
        std::cerr << "arg error: " << e.what() << std::endl;
    }

    return 0;
}
