# Changelog

All notable changes to xleak. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and the project uses [Semantic Versioning](https://semver.org/).

## [0.1.0] - 2026-11-04

### Added
- The project: a static library with one header, for use with `add_subdirectory` or `find_package`.
- Linking the library and including `xleak/xleak.h` in one file starts xleak before the program's
  global objects are constructed, on Windows with MSVC and on Linux with glibc.
- `xleak::is_active`, and the environment variable `XLEAK=off` to turn it off when a program runs.

xleak watches no allocations yet.
