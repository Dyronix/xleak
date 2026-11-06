// Prints when each part of a program runs, so CTest can check where xleak sits around them.
#include "xleak/xleak.h"

#include <cstdio>

namespace
{
    // Unbuffered, like xleak's own output, so the lines keep their order
    void say(const char* text)
    {
        std::fputs(text, stderr);
        std::fputs("\n", stderr);
    }

    struct global_object
    {
        global_object()
        {
            say("global constructed");
        }

        ~global_object()
        {
            say("global destroyed");
        }
    };

    struct thread_object
    {
        thread_object()
        {
            say("thread_local constructed");
        }

        ~thread_object()
        {
            say("thread_local destroyed");
        }

        void use() const
        {
        }
    };

    // convention-exception: test-only, the point is a global's construction and destruction
    const global_object g_object;
} // namespace

//-------------------------------------------------------------------------
int main()
{
    say("main");

    thread_local const thread_object thread;
    thread.use();

    return 0;
}
