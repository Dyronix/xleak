#include "test_check.h"

#include <cstdio>

namespace xleak_test
{
    //-------------------------------------------------------------------------
    check::check()
        : m_failure_count(0)
    {
    }

    //-------------------------------------------------------------------------
    void check::expect(bool condition, std::string_view what, std::source_location where)
    {
        if (condition)
        {
            return;
        }

        ++m_failure_count;
        std::printf("    FAILED %s:%u: %.*s\n", where.file_name(), static_cast<unsigned>(where.line()), static_cast<int>(what.size()), what.data());
    }

    //-------------------------------------------------------------------------
    int32_t check::get_failure_count() const
    {
        return m_failure_count;
    }

    //-------------------------------------------------------------------------
    int32_t run_groups(std::span<const test_group> groups, std::string_view filter)
    {
        // Groups need different environments (XLEAK unset, XLEAK=off), so they run one at a time
        if (filter.empty())
        {
            std::printf("pass the name of the group to run\n");
            return 1;
        }

        int32_t failed_tests = 0;
        int32_t run_tests = 0;
        for (const test_group& group : groups)
        {
            if (group.name != filter)
            {
                continue;
            }

            for (const test_case& test : group.tests)
            {
                std::printf("[%.*s] %.*s\n", static_cast<int>(group.name.size()), group.name.data(), static_cast<int>(test.name.size()), test.name.data());
                const check result = test.run();
                ++run_tests;
                if (result.get_failure_count() > 0)
                {
                    ++failed_tests;
                }
            }
        }

        std::printf("%d tests, %d failed\n", run_tests, failed_tests);
        if (run_tests == 0)
        {
            std::printf("no group is named '%.*s'\n", static_cast<int>(filter.size()), filter.data());
            return 1;
        }

        return failed_tests == 0 ? 0 : 1;
    }
} // namespace xleak_test
