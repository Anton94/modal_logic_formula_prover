#pragma once

#include <functional>
#include <exception>

class TerminationException : public std::exception
{
    const char* what() const noexcept override { return "Termination exception"; }
};

using termination_callback = std::function<bool()>;
void set_termination_callback(const termination_callback& c);

// If a termination is requested throws TerminationException.

extern thread_local termination_callback __is_termination_requested;
// TODO: We can add some other info, like file&line of termination, etc.
#define TERMINATE_IF_NEEDED()                                       \
    if (__is_termination_requested && __is_termination_requested()) \
    {                                                               \
        throw TerminationException();                               \
    }
