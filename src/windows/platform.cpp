#include "platform.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstring>

namespace xleak::internal
{
    //-------------------------------------------------------------------------
    void write_output(const char* text)
    {
        DWORD written = 0;
        WriteFile(GetStdHandle(STD_ERROR_HANDLE), text, static_cast<DWORD>(std::strlen(text)), &written, nullptr);

        if (IsDebuggerPresent())
        {
            OutputDebugStringA(text);
        }
    }

    //-------------------------------------------------------------------------
    bool is_turned_off_by_environment()
    {
        char value[8] = {};
        const DWORD length = GetEnvironmentVariableA("XLEAK", value, sizeof(value));
        return length == 3 && _stricmp(value, "off") == 0;
    }
} // namespace xleak::internal
