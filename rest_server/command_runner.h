#pragma once

#include <string>

namespace command_runner
{

/// A blocking operation which executes @cmd and waits for it to finish. Returns true if @cmd successfully
/// finishes.
/// Writes the produced output to @output.
/// Note that the error output is redirected, so an '2>&1' is appended to the command (@cmd)!
/// Example: "ls -la"
bool run(const std::string& cmd, std::string& output);
}
