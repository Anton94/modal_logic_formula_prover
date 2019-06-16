#include <iostream>
#include <string>

#include "nlohmann_json\json.hpp"
#include "cmd_options\cxxopts.hpp"

using json = nlohmann::json;

int main(int argc, char* argv[])
{
	try
	{
		cxxopts::Options options("FormulaProover", "One line description of MyProgram");

		options.add_options()
			("h,help", "Print help")
			("f,formula", "Formula in json fomrat", cxxopts::value<std::string>())
			;

		auto result = options.parse(argc, argv);

		if (result.count("help"))
		{
			std::cout << options.help() << std::endl;
			return 0;
		}

		if (result.count("formula"))
		{
			auto formula_arg = result["formula"].as<std::string>();

			std::cout << "Parsing forumla: " << formula_arg << std::endl;
			json formula = json::parse(formula_arg);
			std::cout << formula.dump(4) << std::endl;
		}
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}