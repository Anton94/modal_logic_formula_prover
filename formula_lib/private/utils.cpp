#include "utils.h"
#include <numeric>

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

uint64_t to_int(const std::vector<bool>& v)
{
    return std::accumulate(v.begin(), v.end(), 0ull, [](uint64_t acc, bool bit) {
        return (acc << 1) + bit;
    });
}
