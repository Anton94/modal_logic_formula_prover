#pragma once

#include <functional>
#include <sstream>
#include <string>

using logger_func_t = std::function<void(const std::string&)>;

void set_trace_logger(const logger_func_t& f);
void set_info_logger(const logger_func_t& f);
void set_error_logger(const logger_func_t& f);

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
            s_ << v;
        }
        return *this;
    }

private:
    const logger_func_t& f_;
    std::stringstream s_;
};

auto trace() -> logger;
auto info() -> logger;
auto error() -> logger;
