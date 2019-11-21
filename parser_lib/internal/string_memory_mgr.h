#pragma once

#include <string>

// Yes, it could be done much better, but it's more of a workaround and will be removed

std::string* create_lexer_string(const char* s, int len);
void free_lexer_string(std::string* s);
void free_lexer_strings();
size_t get_lexer_strings_size();
