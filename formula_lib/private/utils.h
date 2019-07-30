#pragma once

#include <functional>

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
