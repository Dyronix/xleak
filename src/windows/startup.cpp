#include "session.h"

// init_seg(lib) is MSVC's documented way to construct a library's objects before the program's. It
// may appear once in a file, which is why this file holds nothing else. C4073 only tells us that
// this is what the pragma does, and it would stop a build that treats warnings as errors.
#pragma warning(disable : 4073)
#pragma init_seg(lib)

namespace
{
    struct early_start
    {
        early_start()
        {
            xleak::internal::start();
        }
    };

    // convention-exception: the only way to run before the program's global objects
    const early_start g_early_start;
} // namespace

// The header refers to this, which keeps this file, and so g_early_start, in the program.
extern "C"
{
    int xleak_link_anchor = 1;
}
