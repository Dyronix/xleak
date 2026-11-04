cmake_minimum_required(VERSION 3.21)

# The version lives in VERSION so the release notes, the package config and Doxygen (through the
# XLEAK_VERSION environment variable) read one value.
file(READ "${CMAKE_CURRENT_LIST_DIR}/VERSION" xleak_version)
string(STRIP "${xleak_version}" xleak_version)

project(xleak
    VERSION ${xleak_version}
    DESCRIPTION "A memory leak detector: link it, include one header, read the report"
    HOMEPAGE_URL "https://github.com/Dyronix/xleak"
    LANGUAGES CXX)

# Tests, installing and strict warnings are on only when xleak is the top-level project, so a
# project that vendors it with add_subdirectory gets the library and nothing else.
option(XLEAK_BUILD_TESTS "Build the xleak tests and register them with CTest" ${PROJECT_IS_TOP_LEVEL})
option(XLEAK_INSTALL "Generate the xleak install and package export rules" ${PROJECT_IS_TOP_LEVEL})
option(XLEAK_STRICT_WARNINGS "Compile xleak with -Wall -Wextra -Wpedantic (/W4 on MSVC)" ${PROJECT_IS_TOP_LEVEL})
option(XLEAK_WARNINGS_AS_ERRORS "Treat xleak compiler warnings as errors" OFF)

# Windows needs MSVC's init_seg, and Linux glibc's symbol lookup. Anywhere else the library is the
# header alone, whose functions do nothing, so a project that vendors xleak still builds.
if(MSVC)
    set(xleak_platform windows)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(xleak_platform linux)
else()
    set(xleak_platform "")
    message(STATUS "xleak: ${CMAKE_SYSTEM_NAME} is not supported, so it watches nothing here")
endif()

include(GNUInstallDirs)

if(xleak_platform)
    add_library(xleak STATIC)

    target_sources(xleak
        PRIVATE
            src/platform.h
            src/session.cpp
            src/session.h
            src/${xleak_platform}/platform.cpp
            src/${xleak_platform}/startup.cpp
            include/xleak/xleak.h
    )

    target_include_directories(xleak
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # Set by linking: the header does nothing in a program that doesn't link xleak.
    target_compile_definitions(xleak
        PUBLIC XLEAK_ENABLED
        PRIVATE XLEAK_VERSION_TEXT="${PROJECT_VERSION}")

    # A static library only adds the object files something refers to, and nothing refers to the
    # one that starts xleak. Asking the linker for its anchor keeps it for every program that links
    # this target, whether or not it includes the header. 32-bit MSVC decorates C names with "_".
    if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_link_options(xleak INTERFACE "/INCLUDE:_xleak_link_anchor")
    elseif(MSVC)
        target_link_options(xleak INTERFACE "/INCLUDE:xleak_link_anchor")
    else()
        target_link_options(xleak INTERFACE "LINKER:-u,xleak_link_anchor")
    endif()

    # Private: xleak.h stays plain, so a program isn't forced up to the C++ version xleak uses.
    target_compile_features(xleak PRIVATE cxx_std_20)

    set_target_properties(xleak PROPERTIES
        CXX_EXTENSIONS OFF
        FOLDER "ThirdParty/xleak"
        VERSION ${PROJECT_VERSION}
    )

    # Applied to the library and the tests, never to the targets of a project that vendors xleak.
    add_library(xleak_warnings INTERFACE)
    if(XLEAK_STRICT_WARNINGS)
        target_compile_options(xleak_warnings INTERFACE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>)
    endif()
    if(XLEAK_WARNINGS_AS_ERRORS)
        target_compile_options(xleak_warnings INTERFACE
            $<$<CXX_COMPILER_ID:MSVC>:/WX>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>)
    endif()
    target_link_libraries(xleak PRIVATE $<BUILD_INTERFACE:xleak_warnings>)
else()
    add_library(xleak INTERFACE)
    target_include_directories(xleak INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
endif()
add_library(xleak::xleak ALIAS xleak)

if(XLEAK_INSTALL)
    include(CMakePackageConfigHelpers)

    install(TARGETS xleak
        EXPORT xleak-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(DIRECTORY include/xleak DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES LICENSE README.md CHANGELOG.md DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/xleak)

    set(xleak_config_dir ${CMAKE_INSTALL_LIBDIR}/cmake/xleak)
    install(EXPORT xleak-targets
        NAMESPACE xleak::
        DESTINATION ${xleak_config_dir})
    configure_package_config_file(cmake/xleak-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/xleak-config.cmake
        INSTALL_DESTINATION ${xleak_config_dir})
    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/xleak-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/xleak-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/xleak-config-version.cmake
        DESTINATION ${xleak_config_dir})
endif()

if(XLEAK_BUILD_TESTS AND xleak_platform)
    enable_testing()
    add_subdirectory(tests)
endif()
