cmake_minimum_required(VERSION 3.21)
project(xleak_spike LANGUAGES CXX)

# M0 throwaway. The hooks are a STATIC library, as the plan says, so this proves the archive members
# are pulled in and that the exe's definitions win for libc, libstdc++ and later dlopen'ed libraries.
add_library(spike_hooks STATIC startup.cpp malloc_hooks.cpp new_hooks.cpp)
target_compile_features(spike_hooks PUBLIC cxx_std_20)
target_compile_options(spike_hooks PRIVATE -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free)
target_link_libraries(spike_hooks PUBLIC ${CMAKE_DL_LIBS})

add_library(spike_linked SHARED linked.cpp)
add_library(spike_loaded MODULE loaded.cpp)

add_executable(spike main.cpp)
target_link_libraries(spike PRIVATE spike_hooks spike_linked)
add_dependencies(spike spike_loaded)
set_target_properties(spike PROPERTIES BUILD_RPATH "$ORIGIN" CXX_EXTENSIONS OFF)
target_compile_definitions(spike PRIVATE "SPIKE_LOADED_PATH=\"$<TARGET_FILE:spike_loaded>\"")

foreach(target spike_hooks spike_linked spike_loaded spike)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endforeach()
