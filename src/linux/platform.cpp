#include "platform.h"

#include <strings.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>

namespace xleak::internal
{
    //-------------------------------------------------------------------------
    void write_output(const char* text)
    {
        const std::size_t length = std::strlen(text);

        // Nothing useful can be done when the error output is gone
        [[maybe_unused]] const ssize_t written = write(STDERR_FILENO, text, length);
    }

    //-------------------------------------------------------------------------
    bool is_turned_off_by_environment()
    {
        const char* const value = std::getenv("XLEAK");
        return value != nullptr && strcasecmp(value, "off") == 0;
    }
} // namespace xleak::internal
