#pragma once

#include <cstdint>
#include <source_location>
#include <span>
#include <string_view>

// A tiny test runner, so xleak needs no test framework and no download at configure time.
// Usage: a test is a function that makes a check, calls expect() on it and returns it.
//     xleak_test::check an_example() { xleak_test::check c; c.expect(...); return c; }
// Single threaded: a check belongs to the thread that made it.

namespace xleak_test
{
    // Collects the failures of one test and prints each one when it happens.
    class check
    {
    public:
        check();

        // Records a failure, with the caller's file and line, when condition is false.
        void expect(bool condition, std::string_view what, std::source_location where = std::source_location::current());

        int32_t get_failure_count() const;

    private:
        int32_t m_failure_count;
    };

    // One named test.
    struct test_case
    {
        std::string_view name;
        check (*run)();
    };

    // One group of tests, run by name from the command line.
    struct test_group
    {
        std::string_view name;
        std::span<const test_case> tests;
    };

    // Runs every test of the groups whose name matches filter (all groups for an empty filter).
    // Returns the process exit code: 0 when every test passed.
    int32_t run_groups(std::span<const test_group> groups, std::string_view filter);

    std::span<const test_case> get_start_tests();
    std::span<const test_case> get_turned_off_tests();
    std::span<const test_case> get_starting_twice_tests();
} // namespace xleak_test
