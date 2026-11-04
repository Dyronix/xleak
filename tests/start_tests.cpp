#include "session.h"
#include "test_check.h"
#include "xleak/xleak.h"

#include <array>

namespace
{
    using xleak_test::check;

    // convention-exception: test-only, the point is to read this during global construction
    const bool g_active_during_global_construction = xleak::is_active();

    //-------------------------------------------------------------------------
    check it_is_active_in_main()
    {
        check c;
        c.expect(xleak::is_active(), "active");

        return c;
    }

    //-------------------------------------------------------------------------
    check it_is_active_before_global_objects_are_constructed()
    {
        check c;
        c.expect(g_active_during_global_construction, "active during global construction");

        return c;
    }

    //-------------------------------------------------------------------------
    check turning_it_off_keeps_it_inactive()
    {
        check c;
        c.expect(!xleak::is_active(), "inactive in main");
        c.expect(!g_active_during_global_construction, "inactive during global construction");

        return c;
    }

    //-------------------------------------------------------------------------
    check a_second_start_changes_nothing()
    {
        check c;
        xleak::internal::start();
        c.expect(xleak::is_active(), "still active");

        return c;
    }

    constexpr std::array k_start_tests = {
        xleak_test::test_case{.name = "it_is_active_in_main", .run = it_is_active_in_main},
        xleak_test::test_case{.name = "it_is_active_before_global_objects_are_constructed", .run = it_is_active_before_global_objects_are_constructed},
    };

    constexpr std::array k_turned_off_tests = {
        xleak_test::test_case{.name = "turning_it_off_keeps_it_inactive", .run = turning_it_off_keeps_it_inactive},
    };

    constexpr std::array k_starting_twice_tests = {
        xleak_test::test_case{.name = "a_second_start_changes_nothing", .run = a_second_start_changes_nothing},
    };
} // namespace

namespace xleak_test
{
    //-------------------------------------------------------------------------
    std::span<const test_case> get_start_tests()
    {
        return k_start_tests;
    }

    //-------------------------------------------------------------------------
    std::span<const test_case> get_turned_off_tests()
    {
        return k_turned_off_tests;
    }

    //-------------------------------------------------------------------------
    std::span<const test_case> get_starting_twice_tests()
    {
        return k_starting_twice_tests;
    }
} // namespace xleak_test
