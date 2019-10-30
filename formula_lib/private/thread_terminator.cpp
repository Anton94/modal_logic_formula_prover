#include "../include/thread_termiator.h"

thread_local termination_callback __is_termination_requested;

void set_termination_callback(const termination_callback& c)
{
    __is_termination_requested = c;
}
