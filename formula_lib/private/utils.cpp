#include "utils.h"

call_on_destroy::call_on_destroy(std::function<void()>&& callback)
    : callback_(std::move(callback))
{
}

call_on_destroy::~call_on_destroy()
{
    if(callback_)
    {
        callback_();
    }
}
