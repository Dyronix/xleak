#pragma once

// The one running detector of this process.

namespace xleak::internal
{
    // Called once by the platform's startup, before the program's global objects are constructed.
    // A second call does nothing.
    void start();
} // namespace xleak::internal
