# Input Module Tests

cmake_minimum_required(VERSION 3.15)

set(TEST_NAME LuaUI_Input_Tests)

file(GLOB TEST_SOURCES "*.cpp")

if(NOT TEST_SOURCES)
    message(STATUS "No tests found for ${TEST_NAME}, skipping")
    return()
endif()

add_executable(${TEST_NAME} ${TEST_SOURCES})

target_link_libraries(${TEST_NAME} PRIVATE
    LuaUI_Input
    Catch2::Catch2WithMain
)

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
