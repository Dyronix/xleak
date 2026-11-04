#include "session.h"

#include "platform.h"
#include "xleak/xleak.h"

#include <atomic>
#include <cstdlib>

namespace
{
    // convention-exception: hooks are process-wide and start before any caller exists to own them
    std::atomic<bool> g_is_active{false};

    //-------------------------------------------------------------------------
    // Runs when the program exits. Registered before the program's global objects are constructed,
    // so atexit's last-in, first-out order runs it after all of their destructors.
    void finish()
    {
        xleak::internal::write_output("xleak: finished.\n");
    }
} // namespace

namespace xleak::internal
{
    //-------------------------------------------------------------------------
    void start()
    {
        if (is_turned_off_by_environment() || g_is_active.exchange(true))
        {
            return;
        }

        write_output("xleak: started (" XLEAK_VERSION_TEXT "). No allocations are watched yet.\n");
        std::atexit(finish);
    }
} // namespace xleak::internal

namespace xleak
{
    //-------------------------------------------------------------------------
    bool is_active()
    {
        return g_is_active;
    }
} // namespace xleak
