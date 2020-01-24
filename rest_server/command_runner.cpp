#include "command_runner.h"

#include <cstdio>

namespace command_runner
{

void read_output(FILE* file, std::string& output)
{
    char byte{0};
    while(fread(&byte, 1, 1, file))
    {
        output.push_back(byte);
    }
}

bool run(const std::string& cmd, std::string& output)
{
#ifdef __WIN32__
#define popen  _popen
#endif
    auto cmd_with_redirected_error_output = cmd + " 2>&1";
    FILE* file = popen(cmd_with_redirected_error_output.c_str(), "r");
    if(!file)
    {
        return false;
    }

    read_output(file, output);

    return pclose(file) == 0;
}
}
