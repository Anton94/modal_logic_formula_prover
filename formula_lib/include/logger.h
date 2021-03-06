#pragma once

#include <functional>
#include <sstream>
#include <string>

using logger_func_t = std::function<void(std::stringstream&&)>;

class logger
{
public:
    logger(const logger_func_t& f);
    ~logger();

    template <typename T>
    logger& operator<<(T&& v)
    {
        if(f_)
        {
            s_ << std::forward<T>(v);
        }
        return *this;
    }

    auto get_buff() -> std::stringstream&;
    auto is_enabled() const -> bool;

private:
    const logger_func_t& f_;
    std::stringstream s_;
};

void set_verbose_logger(const logger_func_t& f);
void set_trace_logger(const logger_func_t& f);
void set_info_logger(const logger_func_t& f);
void set_error_logger(const logger_func_t& f);

void set_verbose_logger(logger_func_t&& f);
void set_trace_logger(logger_func_t&& f);
void set_info_logger(logger_func_t&& f);
void set_error_logger(logger_func_t&& f);

auto verbose() -> logger;
auto trace() -> logger;
auto info() -> logger;
auto error() -> logger;
