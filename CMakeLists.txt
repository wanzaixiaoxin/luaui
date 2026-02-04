cmake_minimum_required(VERSION 3.15)
project(LuaUI VERSION 1.0.0 LANGUAGES CXX)

# 设置C++11标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /MT)
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /GL /DNDEBUG")
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找第三方库
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/lua
    ${CMAKE_SOURCE_DIR}/third_party/tinyxml2
)

# 子目录
add_subdirectory(third_party/lua)
add_subdirectory(third_party/tinyxml2)

# 核心库
add_subdirectory(src/core)

# XML模块
add_subdirectory(src/xml)

# UI模块
add_subdirectory(src/ui)

# Lua模块
add_subdirectory(src/lua)

# 资源模块
add_subdirectory(src/resource)

# 工具模块
add_subdirectory(src/utils)

# 示例应用
add_subdirectory(examples/hello_world)
add_subdirectory(examples/notepad)

# 安装配置
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY docs/ DESTINATION docs/share/luaui)
install(TARGETS LuaUI DESTINATION lib)
