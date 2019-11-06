#include "task_info.h"

task_info::task_info()
{
    result_.status_code = "RUNNING";
    activate();
}

pplx::cancellation_token task_info::get_token()
{
    return cts_.get_token();
}

void task_info::cancel()
{
    cts_.cancel();
}

bool task_info::is_active()
{
    return is_active_;
}

void task_info::activate()
{
    is_active_ = true;
}

void task_info::deactivate()
{
    is_active_ = false;
}
