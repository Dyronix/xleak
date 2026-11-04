add_executable(xleak_tests
    main.cpp
    start_tests.cpp
    test_check.cpp
    test_check.h
)
target_link_libraries(xleak_tests PRIVATE xleak::xleak xleak_warnings)
target_include_directories(xleak_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_features(xleak_tests PRIVATE cxx_std_20)
set_target_properties(xleak_tests PROPERTIES CXX_EXTENSIONS OFF FOLDER "ThirdParty/xleak")

# One CTest entry per group, so a failure names the area. The runner takes the group name.
add_test(NAME xleak.start COMMAND xleak_tests start)
add_test(NAME xleak.turned_off COMMAND xleak_tests turned_off)
set_tests_properties(xleak.turned_off PROPERTIES ENVIRONMENT "XLEAK=off")

# A second start must not register a second exit handler.
add_test(NAME xleak.starting_twice COMMAND xleak_tests starting_twice)
set_tests_properties(xleak.starting_twice PROPERTIES FAIL_REGULAR_EXPRESSION "finished.*finished")

# What a program sees from outside: these end by exiting, so CTest reads their output.
function(xleak_add_program name)
    add_executable(${name} ${name}.cpp)
    target_compile_features(${name} PRIVATE cxx_std_20)
    target_link_libraries(${name} PRIVATE xleak_warnings)
    set_target_properties(${name} PROPERTIES CXX_EXTENSIONS OFF FOLDER "ThirdParty/xleak")
endfunction()

# Including the header is all a linked program does, and that starts xleak.
xleak_add_program(include_only_program)
target_link_libraries(include_only_program PRIVATE xleak::xleak)
add_test(NAME xleak.including_the_header_is_enough COMMAND include_only_program)
set_tests_properties(xleak.including_the_header_is_enough PROPERTIES PASS_REGULAR_EXPRESSION "xleak: started")

# Started before the program's global objects, finished after them.
xleak_add_program(order_program)
target_link_libraries(order_program PRIVATE xleak::xleak)
add_test(NAME xleak.order_around_the_program COMMAND order_program)
set_tests_properties(xleak.order_around_the_program PROPERTIES
    PASS_REGULAR_EXPRESSION "xleak: started.*global constructed.*main.*thread_local destroyed.*global destroyed.*xleak: finished")

add_test(NAME xleak.turned_off_prints_nothing COMMAND order_program)
set_tests_properties(xleak.turned_off_prints_nothing PROPERTIES
    ENVIRONMENT "XLEAK=off"
    FAIL_REGULAR_EXPRESSION "xleak:")

# The header in a program that doesn't link the library: it builds, and nothing runs.
xleak_add_program(not_linked_program)
target_include_directories(not_linked_program PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME xleak.not_linked_does_nothing COMMAND not_linked_program)
set_tests_properties(xleak.not_linked_does_nothing PROPERTIES FAIL_REGULAR_EXPRESSION "xleak:")
