#pragma once

/// @file
/// @brief The one header of xleak. Include it in any one source file of a program that links xleak.
///
/// Linking the xleak::xleak CMake target is enough to start xleak. For a build that links the
/// library some other way, including the header is what makes the linker keep it, the same way
/// Visual Leak Detector's header does. In a program that doesn't link xleak it compiles to nothing, so the include can
/// stay in the code.
///
/// @code
/// #include "xleak/xleak.h"
/// @endcode
///
/// Set the environment variable XLEAK to `off` to run a linked program without detection.
///
/// Threading: everything in this file is safe to call from any thread.

#if defined(XLEAK_ENABLED)
// A static library only adds the object files something refers to, and nothing refers to the one
// that starts xleak before global objects are constructed. This reference is what keeps it.
extern "C" int xleak_link_anchor;
#if defined(_MSC_VER) && defined(_M_IX86)
// 32-bit MSVC decorates C names with a leading underscore
#pragma comment(linker, "/include:_xleak_link_anchor")
#elif defined(_MSC_VER)
#pragma comment(linker, "/include:xleak_link_anchor")
#else
[[gnu::used]] static const int* const xleak_link_anchor_reference = &xleak_link_anchor;
#endif
#endif

namespace xleak
{
#if defined(XLEAK_ENABLED)
    /// @brief Whether xleak is running in this program.
    /// @return True from before the program's global objects are constructed. False when the
    /// program doesn't link xleak, on a platform it doesn't support, or with XLEAK set to `off`.
    bool is_active();
#else
    // Its own namespace gives this a different name from the library's is_active, so a file built
    // without xleak can't clash with it when both end up in one program.
    inline namespace not_linked
    {
        /// @brief Whether xleak is running in this program.
        /// @return True from before the program's global objects are constructed. False when the
        /// program doesn't link xleak, on a platform it doesn't support, or with XLEAK set to `off`.
        inline bool is_active()
        {
            return false;
        }
    } // namespace not_linked
#endif
} // namespace xleak
