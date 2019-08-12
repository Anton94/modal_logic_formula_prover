#pragma once

#include <functional>
#include <vector>

class call_on_destroy
{
public:
    call_on_destroy(std::function<void()>&& callback);
    ~call_on_destroy();

    call_on_destroy(const call_on_destroy&) = delete;
    call_on_destroy& operator=(const call_on_destroy&) = delete;
    call_on_destroy(call_on_destroy&&) = delete;
    call_on_destroy& operator=(call_on_destroy&&) = delete;

private:
    std::function<void()> callback_;
};

uint64_t to_int(const std::vector<bool>& v);
