cmake_minimum_required(VERSION 3.18)
project(RecoverySoftNetz VERSION 0.1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_DOCS "Build documentation" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Find packages
find_package(Qt6 COMPONENTS Core Widgets QUIET)
find_package(GTest QUIET)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(CORE_SOURCES
    src/core/recovery_engine.cpp
    src/core/file_registry.cpp
    src/core/file_carving.cpp
)

set(COMMON_SOURCES
    src/common/logging.cpp
    src/common/utils.cpp
)

set(FILESYSTEM_SOURCES
    src/filesystems/ntfs_parser.cpp
    src/filesystems/apfs_parser.cpp
    src/filesystems/ext4_parser.cpp
)

set(CLI_SOURCES
    src/cli/cli.cpp
)

# Core library
add_library(rsn_core STATIC
    ${CORE_SOURCES}
    ${COMMON_SOURCES}
    ${FILESYSTEM_SOURCES}
    ${CLI_SOURCES}
)

target_include_directories(rsn_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# Qt UI (optional - only if Qt6 found)
if(Qt6_FOUND)
    message(STATUS "Qt6 found - building UI components")

    set(UI_SOURCES
        src/ui/main_window.cpp
        src/ui/device_wizard.cpp
    )

    add_library(rsn_ui STATIC ${UI_SOURCES})
    target_link_libraries(rsn_ui
        Qt6::Core
        Qt6::Widgets
        rsn_core
    )
else()
    message(WARNING "Qt6 not found - UI components will not be built")
endif()

# Main executable (requires Qt)
if(Qt6_FOUND)
    add_executable(rsn src/main.cpp)
    target_link_libraries(rsn
        rsn_core
        rsn_ui
        Qt6::Core
        Qt6::Widgets
    )
endif()

# Tests
if(BUILD_TESTS AND GTest_FOUND)
    message(STATUS "Building tests with GoogleTest")
    enable_testing()

    add_executable(rsn_tests
        tests/unit/core/recovery_engine_test.cpp
        tests/unit/core/file_carving_test.cpp
        tests/unit/filesystems/ntfs_parser_test.cpp
        tests/unit/filesystems/ntfs_structures_test.cpp
        tests/unit/common/utils_test.cpp
    )

    target_link_libraries(rsn_tests
        rsn_core
        GTest::gtest
        GTest::gtest_main
        pthread
    )

    # Add test
    add_test(NAME rsn_unit_tests COMMAND rsn_tests)

    # Coverage (optional)
    if(ENABLE_COVERAGE)
        target_compile_options(rsn_core PRIVATE --coverage)
        target_link_libraries(rsn_core PRIVATE --coverage)
    endif()
elseif(BUILD_TESTS AND NOT GTest_FOUND)
    message(WARNING "GTest not found - tests will not be built")
endif()

# Installation
install(TARGETS rsn_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

if(Qt6_FOUND)
    install(TARGETS rsn
        RUNTIME DESTINATION bin
    )
endif()

# Documentation (Doxygen)
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        add_custom_target(docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else()
        message(WARNING "Doxygen not found - documentation will not be built")
    endif()
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "===== RecoverySoftNetz Configuration =====")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Qt6 found: ${Qt6_FOUND}")
message(STATUS "GTest found: ${GTest_FOUND}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Enable coverage: ${ENABLE_COVERAGE}")
message(STATUS "==========================================")
message(STATUS "")
