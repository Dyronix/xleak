cmake_minimum_required(VERSION 3.21)
project(xleak_vendored LANGUAGES CXX)

# xleak's tests stay out of a project that vendors it, so CTest finds only this project's test.
enable_testing()
add_subdirectory(../../.. xleak)

add_executable(consumer ../main.cpp)
target_link_libraries(consumer PRIVATE xleak::xleak)
add_test(NAME consumer COMMAND consumer)
