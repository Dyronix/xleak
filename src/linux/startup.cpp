#include "session.h"

namespace
{
    // convention-exception: the only way to run before the program's global objects. Priorities up
    // to 100 are reserved for the implementation, so 101 is the earliest a library may ask for.
    __attribute__((constructor(101))) void early_start()
    {
        xleak::internal::start();
    }
} // namespace

// The header refers to this, which keeps this file, and so early_start, in the program.
extern "C"
{
    int xleak_link_anchor = 1;
}
