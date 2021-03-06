#include "logger.h"

namespace
{
thread_local logger_func_t verbose_logger;
thread_local logger_func_t trace_logger;
thread_local logger_func_t info_logger;
thread_local logger_func_t error_logger;
}

void set_verbose_logger(const logger_func_t& f)
{
    verbose_logger = f;
}

void set_trace_logger(const logger_func_t& f)
{
    trace_logger = f;
}

void set_info_logger(const logger_func_t& f)
{
    info_logger = f;
}

void set_error_logger(const logger_func_t& f)
{
    error_logger = f;
}

void set_verbose_logger(logger_func_t&& f)
{
    verbose_logger = std::move(f);
}

void set_trace_logger(logger_func_t&& f)
{
    trace_logger = std::move(f);
}

void set_info_logger(logger_func_t&& f)
{
    info_logger = std::move(f);
}

void set_error_logger(logger_func_t&& f)
{
    error_logger = std::move(f);
}

logger::logger(const logger_func_t& f)
    : f_(f)
{
}

logger::~logger()
{
    if(f_ && s_.rdbuf()->in_avail() > 0)
    {
        f_(std::move(s_));
    }
}

auto logger::get_buff() -> std::stringstream&
{
    return s_;
}

auto logger::is_enabled() const -> bool
{
    return f_ != nullptr;
}

auto verbose() -> logger
{
    return {verbose_logger};
}

auto trace() -> logger
{
    return {trace_logger};
}

auto info() -> logger
{
    return {info_logger};
}

auto error() -> logger
{
    return {error_logger};
}
