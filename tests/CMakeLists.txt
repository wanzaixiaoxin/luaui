cmake_minimum_required(VERSION 3.15)

# Simple test executable
add_executable(simple_test simple_test.cpp)
target_link_libraries(simple_test PRIVATE tinyxml2)

# Add as CTest
add_test(NAME SimpleTest COMMAND simple_test)
