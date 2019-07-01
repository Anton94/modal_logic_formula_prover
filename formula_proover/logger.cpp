#include "logger.h"

namespace
{

auto get_trace_logger_func() -> logger_func_t&
{
    static logger_func_t logger_func;
    return logger_func;
}

auto get_info_logger_func() -> logger_func_t&
{
    static logger_func_t logger_func;
    return logger_func;
}

auto get_error_logger_func() -> logger_func_t&
{
    static logger_func_t logger_func;
    return logger_func;
}
}

void set_trace_logger(const logger_func_t& f)
{
    get_trace_logger_func() = f;
}

void set_info_logger(const logger_func_t& f)
{
    get_info_logger_func() = f;
}

void set_error_logger(const logger_func_t& f)
{
    get_error_logger_func() = f;
}

logger::logger(const logger_func_t& f)
    : f_(f)
{
}

logger::~logger()
{
    if(f_)
    {
        f_(s_.str());
    }
}

auto trace() -> logger
{
    return {get_trace_logger_func()};
}

auto info() -> logger
{
    return {get_info_logger_func()};
}

auto error() -> logger
{
    return {get_error_logger_func()};
}
