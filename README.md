# xleak

A small C++ memory leak detector in the spirit of Visual Leak Detector, for Windows and Linux. Link
the library, include one header, run your program, and read which allocations were never freed and
which call stacks made them. No changes to your allocation code.

**Status: in development. There is no usable code yet.** Everything below describes the design
being built, not something you can use today.

## Planned usage

```cmake
add_subdirectory(xleak)
target_link_libraries(my_game PRIVATE xleak::xleak)
```

```cpp
#include "xleak/xleak.h" // in any one source file
```

Run the program and exit normally. Leaks are grouped by call stack:

```text
=== 2 leak locations, 3 allocations, 192 bytes not freed at exit ===

[1] 2 allocations, 128 bytes (64 each)
  C:\game\player.cpp(42): player::player
  C:\game\game.cpp(88): game::game
  C:\game\main.cpp(26): main

[2] 1 allocation, 64 bytes
  C:\game\inventory.cpp(120): inventory::create_item
  C:\game\player.cpp(51): player::player
```

## Design goals

- One static library and one header. Nothing to copy next to the executable.
- Watches `new`, `delete` and the `malloc` family, including from libraries linked into your
  executable, from before global objects are constructed.
- Honest output: the report states what was watched and what wasn't, and says so when a program
  exited too early for a leak verdict to mean anything.
- Debug symbols give function, file and line. Without them, you get module and offset.

## Planned limits

xleak will not see GPU or driver memory, direct `VirtualAlloc` or `mmap`, or objects inside your
own pools (only the pool's backing block). There is no report if the process crashes or is killed.
Use AddressSanitizer for memory corruption: xleak only reports what wasn't freed.

## Licence

MIT, see `LICENSE`. xleak is written independently and contains no Visual Leak Detector code.
