#include "test_check.h"

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

//-------------------------------------------------------------------------
int main(int argc, char** argv)
{
    const std::array groups = {
        xleak_test::test_group{.name = "start", .tests = xleak_test::get_start_tests()},
        xleak_test::test_group{.name = "turned_off", .tests = xleak_test::get_turned_off_tests()},
        xleak_test::test_group{.name = "starting_twice", .tests = xleak_test::get_starting_twice_tests()},
    };

    const std::span<char*> arguments(argv, static_cast<std::size_t>(argc));
    const std::string_view filter = arguments.size() > 1 ? std::string_view(arguments[1]) : std::string_view();

    return xleak_test::run_groups(groups, filter);
}
