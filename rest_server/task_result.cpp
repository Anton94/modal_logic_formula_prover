#include "task_result.h"

std::string task_result::to_string()
{
    json::value result;
    result[U("status")] = json::value(utility::conversions::to_string_t(status_code));
    result[U("is_satisfied")] = json::value(is_satisfied);
    result[U("contacts")] = json_contants();
    result[U("ids")] = json_ids();
    result[U("variables")] = json_variables();
    result[U("output")] = json::value(utility::conversions::to_string_t(output));

    try
    {
        utility::stringstream_t stream;

        std::string resultingString = utility::conversions::to_utf8string(result.serialize());

        return resultingString;
        // string_t x = result.as_string();
    }
    catch(const std::exception& exc)
    {
        // catch anything thrown within try block that derives from std::exception
        std::string xq = exc.what();
    }
    return utility::conversions::to_utf8string(result.as_string());
}

json::value task_result::json_contants()
{
    json::value result = json::value::array();
    for(size_t i = 0, len_i = contacts.size(); i < len_i; ++i)
    {
        json::value inner_array = json::value::array();
        for(size_t j = 0, len_j = contacts[i].size(); j < len_j; ++j)
        {
            inner_array[j] = json::value(contacts[i][j] == true ? U("1") : U("0"));
        }
        result[i] = inner_array;
    }

    return result;
}

json::value task_result::json_ids()
{
    json::value result = json::value::array();
    for (size_t i = 0, len = ids.size(); i < len; ++i)
    {
        json::value inner_array = json::value::array();
        for (size_t j = 0, len_j = ids[i].size(); j < len_j; ++j)
        {
            inner_array[j] = json::value(ids[i][j] == true ? U("1") : U("0"));
        }
        result[i] = inner_array;
    }

    return result;
}

json::value task_result::json_variables()
{
    json::value result = json::value::array();
    for (size_t i = 0, len = variables.size(); i < len; ++i)
    {
        result[i] = json::value(utility::conversions::to_string_t(variables[i]));
    }

    return result;
}
