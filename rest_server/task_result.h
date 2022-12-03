#pragma once

#include "library.h"
#include <cpprest/json.h>

using namespace web;

class task_result
{
public:
    std::string status_code; // enum "RUNNING", "CANCELED", "FINISHED", ...
    bool is_parsed{};
    bool is_satisfied{};
    contacts_t contacts;
    variable_id_to_points_t ids;
    variables_t variables;
    std::string output;

    std::string to_string();

private:
    json::value json_contants();
    json::value json_ids();
    json::value json_variables();
};
