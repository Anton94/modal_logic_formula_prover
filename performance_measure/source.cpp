#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <fstream>

#include "cmd_options/cxxopts.hpp"
#include "library.h"
#include "thread_termiator.h" // TODO: it's not necessary a thread termination, but only a operation termination, so rename!

using namespace std::chrono_literals;
using time_type = std::chrono::milliseconds;

// TODO: split the file to header, etc.
// TODO: write the output to a file

static const int RUNS_PER_FORMULA = 5; // better to keep it odd
static const time_type TIME_LIMIT_PER_FORMULA = 1500ms;

static const bool DEBUG_LOG_ENABLED = false;

template <typename T>
time_type process_formula_group_for_model(const std::vector<std::string>& formulas)
{
    assert(!formulas.empty());
    static_assert(std::is_same<T, model>::value ||
        std::is_same<T, measured_model>::value ||
        std::is_same<T, connected_model>::value ||
        std::is_same<T, basic_bruteforce_model>::value, "The type T should be one of the model types!");

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<time_type> group_meadian_time_per_formula(formulas.size());
    std::vector<time_type> formula_run_times(RUNS_PER_FORMULA);

    set_termination_callback([&]()
    {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto ellapsed_time = std::chrono::duration_cast<time_type>(now - start);
        return ellapsed_time > TIME_LIMIT_PER_FORMULA;
    });

    for (int i = 0; i < formulas.size(); ++i)
    {
        const auto& f = formulas[i];
        if(DEBUG_LOG_ENABLED) std::cout << "Running: " << f << "\n";

        for (int j = 0; j < RUNS_PER_FORMULA; ++j)
        {
            formula_mgr mgr;
            if (!mgr.build(f))
            {
                std::cerr << "Unable to build formula: " << f << "\n";
                return -1s;
            }
            T m;

            start = std::chrono::high_resolution_clock::now();
            try
            {
                mgr.is_satisfiable(m);
            }
            catch (const TerminationException&)
            {
                if (DEBUG_LOG_ENABLED) std::cout << "Timeouted...\n";
                formula_run_times[j] = TIME_LIMIT_PER_FORMULA;
                continue;
            }
            const auto now = std::chrono::high_resolution_clock::now();
            formula_run_times[j] = std::chrono::duration_cast<time_type>(now - start);

            if (DEBUG_LOG_ENABLED) std::cout << "Elapsed time: " << formula_run_times[j].count() << "ms\n";
        }
        // Will take the meadian time because there might be some CPU spikes which we should not take into an account
        std::sort(formula_run_times.begin(), formula_run_times.end());
        group_meadian_time_per_formula[i] = formula_run_times[formula_run_times.size() / 2];

        if (DEBUG_LOG_ENABLED) std::cout << "Median time: " << group_meadian_time_per_formula[i].count() << "ms\n";
    }

    // TODO: what if some of the formulas timeouts and some of them do not. What should be the output then?

    // Will take the average time because some formulas might take a long time and be in some sort of outliners but they should be taken into account
    time_type sum = 0ms;
    for (const auto& meadin_time : group_meadian_time_per_formula)
    {
        sum += meadin_time;
    }
    assert(group_meadian_time_per_formula.size() > 0);
    const auto average_time = sum / group_meadian_time_per_formula.size();
    return average_time;
}

void process_formula_group(std::ifstream& in, int formulas_per_group)
{
    assert(formulas_per_group > 0);

    // Read the info line
    std::string info_line;
    std::getline(in, info_line);
    if (in.eof())
    {
        std::cout << "Finished!\n";
        return;
    }

    if (!in)
    {
        std::cerr << "Unable to read a group's info line.\n";
        return;
    }

    std::cout << info_line << "\n";

    std::vector<std::string> formulas(formulas_per_group);
    for (int i = 0; i < formulas_per_group; ++i)
    {
        std::getline(in, formulas[i]);
        if (!in)
        {
            std::cerr << "Unable to read the " << i + 1 << "th formula of the group.\n";
            return;
        }
        if(DEBUG_LOG_ENABLED) std::cout << "Read formula: " << formulas[i] << "\n";
    }

    std::cout << "Model took average            : " << process_formula_group_for_model<model>(formulas).count() << "ms\n";
    std::cout << "Connected model took average  : " << process_formula_group_for_model<connected_model>(formulas).count() << "ms\n";
    std::cout << "Measured model took average   : " << process_formula_group_for_model<measured_model>(formulas).count() << "ms\n";
    std::cout << "Brute force model took average: " << process_formula_group_for_model<basic_bruteforce_model>(formulas).count() << "ms\n";
}

int main(int argc, char* argv[])
{
    // set_trace_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    // set_info_logger([](std::stringstream&& s) { std::cout << s.rdbuf() << std::endl; });
    set_error_logger([](std::stringstream&& s) { std::cerr << "ERROR: " << s.rdbuf() << "\n"; });

    try
    {
        cxxopts::Options options("PerformanceMeasurer", "Measures the average time needed by is_satisfiable for each formula group with each model."
            "More precisely, for each formula group - runs each formula " + std::to_string(RUNS_PER_FORMULA) + " times and takes the middle time, then takes the average time of all middle times."
            "Note that there is an upper limit of " + std::to_string(TIME_LIMIT_PER_FORMULA.count()) + "ms for each formula to execute is_satisfiable.\n"
            "Input file format:\n\n"
            "N\n"
            "Some info line describing the group, e.g. number of atomic formulas and variables, etc.\n"
            "C(a,b)&...\n"
            "C(a,b)&...\n"
            "... total of N formulas\n"
            "Next info line describing the next group.\n"
            "...\n\n");

        options.add_options()
            ("h,help", "Print help")
            ("i,input", "File containing the formulas.", cxxopts::value<std::string>());

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help() << "\n";
            return 0;
        }

        if (!result.count("input"))
        {
            std::cerr << "No input file specified. Exiting.\n";
            return 0;
        }

        auto input_file = result["input"].as<std::string>();

        std::cout << "Will read from input file: " << input_file << "\n";

        std::ifstream in(input_file);
        if (!in)
        {
            std::cerr << "Unable to open the file.\n";
            return 0;
        }
        int formulas_per_group = 0;
        in >> formulas_per_group;
        in.get(); // consume the new line
        
        if (!in)
        {
            std::cerr << "Unable to read the number of formulas per group.\n";
            in.close();
            return 0;
        }

        if (formulas_per_group <= 0)
        {
            std::cerr << "Not a valid value for the number of formulas per group.\n";
        }

        std::cout << formulas_per_group << " formulas per group.\n";
        
        while (in)
        {
            process_formula_group(in, formulas_per_group);
        }
        in.close();
    }
    catch (const cxxopts::OptionException& e)
    {
        std::cerr << "arg error: " << e.what();
    }

    return 0;
}
