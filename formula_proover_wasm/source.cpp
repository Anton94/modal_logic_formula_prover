#include <iostream>
#include <string>
#include <fstream>
#include <mutex>

#include "library.h"

#include "nlohmann/json.hpp"

#include <include/emscripten/emscripten.h>

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

namespace
{

inline const char* cstr(const std::string& message)
{
    char * cstr = new char [message.length() + 1]; // + terminating null
    std::strcpy(cstr, message.c_str());
    return cstr; // JS will free it
}

// Ugly but for the purpose of the app is more than enough!
static thread_local std::mutex output_mutex{};

auto get_output_stream() -> std::stringstream&
{
    static std::stringstream output;
    return output;
}

void write_to_output(std::stringstream&& s, const char* prefix = "")
{
    std::lock_guard<std::mutex> op_id_to_ctx_guard(output_mutex);
    get_output_stream() << prefix << s.rdbuf() << "\n";
}

auto get_and_release_accumulated_output() -> std::string
{
    std::lock_guard<std::mutex> op_id_to_ctx_guard(output_mutex);
    auto s = get_output_stream().str();
    get_output_stream() = {};
    return s;
}

}

namespace
{
enum class state
{
    running,
    finished,
};


}

EXTERN EMSCRIPTEN_KEEPALIVE const char* calculate(const char* js_formula, int len)
{
    get_output_stream() = {};

    std::string formula(js_formula, len);

    formula_mgr f;
    const auto correct_formula = f.build(formula);

    info() << "\t" << (correct_formula ? "success" : "failed") << "\n";

    info() << "Getting the used variables...";
    const auto variables = f.get_variables();
    info() << "Variables: " << variables;

    measured_model m{};

    info() << "Trying to find a measured model!";
    const auto res = f.is_satisfiable(m);
    info() << "The formula is " << (res ? "" : "not ") << "satisfiable.";

    auto output = get_output_stream().str();
    get_output_stream() = {};

    std::stringstream out;
    m.print_evaluations(out, m.get_variables_evaluations());
    auto variable_evaluations = out.str();

    out = {};
    m.print_contacts(out, m.get_contact_relations());
    auto contacts = out.str();

    nlohmann::json result;
    result["is_satisfied"] = res;
    result["status"] = "FINISHED";
    result["output"] = std::move(output);
    result["variables"] = f.get_variables();
    result["ids"] = std::move(variable_evaluations);
    result["contacts"] = std::move(contacts);

    const auto as_string = result.dump();

    return cstr(as_string);
}

EXTERN EMSCRIPTEN_KEEPALIVE const char* get_accumulated_output()
{
    std::cout << "Get accumulated output is called!" << std::endl;
    const auto accumulated_output = get_and_release_accumulated_output();
    return cstr(accumulated_output);
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
