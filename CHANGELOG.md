# Changelog

All notable changes to xleak. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and the project uses [Semantic Versioning](https://semver.org/).

## [0.1.0] - 2026-11-04

### Added
- The project: a static library with one header, for use with `add_subdirectory` or `find_package`.
- Linking the `xleak::xleak` CMake target starts xleak before the program's global objects are
  constructed, in 64-bit programs on Windows with MSVC and on Linux with glibc. A build that links
  the library some other way includes `xleak/xleak.h` in one file instead.
- A 32-bit build stops with an error, at configure time and when compiling the header.
- `xleak::is_active`, and the environment variable `XLEAK=off` to turn it off when a program runs.

xleak watches no allocations yet.
