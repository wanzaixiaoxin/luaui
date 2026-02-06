cmake_minimum_required(VERSION 3.15)

# 01_basic_window - Win32 + Direct2D
add_executable(01_basic_window WIN32 01_basic_window.cpp)
target_link_libraries(01_basic_window PRIVATE d2d1)

# 02_hello_lua - Lua integration
add_executable(02_hello_lua 02_hello_lua.cpp)
target_include_directories(02_hello_lua PRIVATE ${CMAKE_SOURCE_DIR}/third_party)

# simple_test - Link against tinyxml2 directly
# (defined in tests/CMakeLists.txt)
