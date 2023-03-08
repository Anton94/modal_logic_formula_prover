#include <iostream>
#include <string>
#include <fstream>

#include "library.h"

#include "nlohmann/json.hpp"

#include <include/emscripten/emscripten.h>

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

using namespace nlohmann;

namespace
{

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

void write_to_output(std::stringstream&& s, const char* prefix = "")
{
    // std::cout << prefix << s.rdbuf() << "\n"; // Uncomment it and all of the output will be printed to the console in the browser
    get_output_stream() << prefix << s.rdbuf() << "\n";
}

auto json_contants(const contacts_t& contacts) -> json
{
    json result = json::array();
    for(size_t i = 0, len_i = contacts.size(); i < len_i; ++i)
    {
        json inner_array = json::array();
        for(size_t j = 0, len_j = contacts[i].size(); j < len_j; ++j)
        {
            inner_array[j] = contacts[i][j] == true ? "1" : "0";
        }
        result[i] = inner_array;
    }

    return result;
}

auto json_ids(const variable_id_to_points_t& ids) -> json
{
    json result = json::array();
    for (size_t i = 0, len = ids.size(); i < len; ++i)
    {
        json inner_array = json::array();
        for (size_t j = 0, len_j = ids[i].size(); j < len_j; ++j)
        {
            inner_array[j] = ids[i][j] == true ? "1" : "0";
        }
        result[i] = inner_array;
    }

    return result;
}

auto extract_formula_refiners(const std::vector<std::string>& formula_filters) -> formula_mgr::formula_refiners
{
    formula_mgr::formula_refiners formula_refs = formula_mgr::formula_refiners::none;

    for(const auto& filter : formula_filters)
    {
        if(filter == "convert_contact_less_eq_with_same_terms")
        {
            formula_refs |= formula_mgr::formula_refiners::convert_contact_less_eq_with_same_terms;
        }
        else if(filter == "convert_disjunction_in_contact_less_eq")
        {
            formula_refs |= formula_mgr::formula_refiners::convert_disjunction_in_contact_less_eq;
        }
        else if(filter == "reduce_constants")
        {
            formula_refs |= formula_mgr::formula_refiners::reduce_constants;
        }
        else if(filter == "reduce_contacts_less_eq_with_constants")
        {
            formula_refs |= formula_mgr::formula_refiners::reduce_contacts_with_constants;
        }
        else if(filter == "remove_double_negation")
        {
            formula_refs |= formula_mgr::formula_refiners::remove_double_negation;
        }
    }

    return formula_refs;
}

auto get_model_from_type(const std::string& algorithm_type) -> std::shared_ptr<imodel>
{
    if(algorithm_type == "MEASURED_MODEL")
    {
        return std::make_shared<measured_model>();
    }

    if(algorithm_type == "FAST_MODEL")
    {
        return std::make_shared<model>();
    }

    if(algorithm_type == "CONNECTIVITY")
    {
        return std::make_shared<connected_model>(10ul);
    }

    if(algorithm_type == "BRUTEFORCE_MODEL")
    {
        return std::make_shared<basic_bruteforce_model>();
    }

    error() << "Something went wrong, received unrecognized model type from JS. "
               "Fallback to FAST MODEL";
    return std::make_shared<model>();
}

void fill_model_extra_data(std::shared_ptr<imodel>& m, json& result)
{
    if(auto measured = std::dynamic_pointer_cast<measured_model>(m))
    {
        result["modal_points_measured_values"] = measured->get_modal_points_measured_values();
    }
}

}

// Expecting the input data to be as json
// Field formula and algorithm type and optional formula filters
EXTERN EMSCRIPTEN_KEEPALIVE const char* calculate(const char* input_data, int len)
{
    get_output_stream() = {};

    std::string input_as_string(input_data, len);
    auto input = json::parse(input_as_string);

    const std::string formula = input["formula"];
    const std::string algorithm_type = input["algorithm_type"];
    const std::vector<std::string> formula_filters = input["formula_filters"];

    const auto formula_refinres = extract_formula_refiners(formula_filters);

    formula_mgr f;
    const auto is_parsed = f.build(input["formula"], formula_refinres);

    info() << "\t" << (is_parsed ? "success" : "failed") << "\n";

    json result;

    result["is_parsed"] = is_parsed;

    if(is_parsed)
    {
        auto m = get_model_from_type(algorithm_type);

        info() << "Getting the used variables...";
        const auto variables = f.get_variables();
        info() << "Variables: " << variables;

        const auto res = f.is_satisfiable(*m);
        info() << "The formula is " << (res ? "" : "not ") << "satisfiable.";

        result["is_satisfied"] = res;
        result["variables"] = f.get_variables();
        result["ids"] = json_ids(m->get_variables_evaluations());
        result["contacts"] = json_contants(m->get_contact_relations());

        fill_model_extra_data(m, result);
    }

    auto output = get_output_stream().str();
    get_output_stream() = {};

    result["status"] = "FINISHED";
    result["output"] = std::move(output);

    const auto as_string = result.dump();

    return cstr(as_string);
}

int main(int argc, char* argv[])
{
    /*
     * This is a special app which is used from JS.
     * Exposes the formula_lib functionality.
     * It is Converted to WebAssembly and gives an API which is used by JS.
     *
     */

    set_trace_logger([](std::stringstream&& s) { write_to_output(std::move(s)); });
    set_info_logger([](std::stringstream&& s) { write_to_output(std::move(s)); });
    set_error_logger([](std::stringstream&& s) { write_to_output(std::move(s), "ERROR: "); });

    return 0;
}
