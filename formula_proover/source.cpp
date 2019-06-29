#include <iostream>
#include <string>

#include "cmd_options/cxxopts.hpp"
#include "nlohmann_json/json.hpp"

#include "formula.h"
#include "tableau.h"

using json = nlohmann::json;

int main(int argc, char* argv[])
{
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

            std::cout << "Parsing forumla: " << formula_arg << std::endl;
            json formula_json = json::parse(formula_arg);
            std::cout << "Parsed into json: \n" << formula_json.dump(4) << std::endl;

            std::cout << "Building a formula tree with the parsed one...\n\t";
            formula f;
            std::cout << (f.build(formula_json) ? "success" : "failed") << " ";
            std::cout << f << std::endl;

            tableau t;
            std::cout << "The formula is " << (t.proof(f) ? "" : "not ") << "a tautology." << std::endl;


        }
    }
    catch(const cxxopts::OptionException& e)
    {
        std::cerr << "arg error: " << e.what() << std::endl;
    }

    return 0;
}
