#include "string_memory_mgr.h"

#include <unordered_set>

static thread_local std::unordered_set<std::string*> created_lexer_strings;

std::string* create_lexer_string(const char* s, int len)
{
    auto str = new (std::nothrow) std::string(s, static_cast<size_t>(len));
    if(!str)
    {
        // TODO: trigger error or something...
        return nullptr;
    }
    created_lexer_strings.insert(str);
    return str;
}

void free_lexer_string(std::string* s)
{
    created_lexer_strings.erase(s);
    delete s;
}

void free_lexer_strings()
{
    for(auto s : created_lexer_strings)
    {
        delete s;
    }
    created_lexer_strings.clear();
}

size_t get_lexer_strings_size()
{
    return created_lexer_strings.size();
}
