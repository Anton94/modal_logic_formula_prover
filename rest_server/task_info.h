#pragma once

#include "library.h"
#include "task_result.h"

#include <chrono>

class task_info
{
private:
    bool is_active_;
    pplx::cancellation_token_source cts_;

public:
    task_result result_;
    std::chrono::milliseconds start{};

    task_info();

public:
    pplx::cancellation_token get_token();
    void cancel();

    bool is_active();
    void activate();
    void deactivate();
};
