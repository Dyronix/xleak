#pragma once

#include <cstddef>

// What differs per operating system, implemented in windows/ and linux/. These can run before main
// and while the program exits, and none of them may allocate.

namespace xleak::internal
{
    // Writes text to the error output, and to the debugger when one is attached.
    void write_output(const char* text);

    // Whether the environment variable XLEAK says not to run.
    bool is_turned_off_by_environment();
} // namespace xleak::internal
