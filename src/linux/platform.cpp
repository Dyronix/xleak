#include "platform.h"

#include <strings.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>

namespace xleak::internal
{
    //-------------------------------------------------------------------------
    void write_output(const char* text)
    {
        std::size_t remaining = std::strlen(text);
        while (remaining > 0)
        {
            const ssize_t written = write(STDERR_FILENO, text, remaining);
            if (written < 0 && errno == EINTR)
            {
                continue;
            }

            // Nothing useful can be done when the error output is gone
            if (written <= 0)
            {
                return;
            }

            text += written;
            remaining -= static_cast<std::size_t>(written);
        }
    }

    //-------------------------------------------------------------------------
    bool is_turned_off_by_environment()
    {
        const char* const value = std::getenv("XLEAK");
        return value != nullptr && strcasecmp(value, "off") == 0;
    }
} // namespace xleak::internal
