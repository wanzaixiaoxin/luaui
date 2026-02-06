cmake_minimum_required(VERSION 3.15)

# Option to disable tests
option(BUILD_TESTS "Build tests" ON)

if(NOT BUILD_TESTS)
    return()
endif()

# Find or fetch Catch2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
    GIT_SHALLOW TRUE
)

# Make Catch2 available
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# Test executable
set(TEST_SOURCES
    test_main.cpp
)

add_executable(LuaUI_Tests ${TEST_SOURCES})

target_link_libraries(LuaUI_Tests PRIVATE
    LuaUI::LuaUI
    Catch2::Catch2WithMain
)

# Discover tests
catch_discover_tests(LuaUI_Tests)
