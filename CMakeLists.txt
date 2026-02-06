cmake_minimum_required(VERSION 3.15)

project(LuaUI
    VERSION 1.0.0
    DESCRIPTION "A Windows UI framework with Lua scripting and Direct2D rendering"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(LUAUI_BUILD_EXAMPLES "Build example applications" ON)
option(LUAUI_BUILD_TESTS "Build tests" OFF)

# Platform check
if(NOT WIN32)
    message(FATAL_ERROR "LuaUI currently only supports Windows")
endif()

# Compiler options for MSVC
if(MSVC)
    add_compile_options(/W4 /MP)
    add_compile_options(/wd4819)  # Code page warning
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(UNICODE _UNICODE)

    # Static runtime linking
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ============================================
# Third-party libraries
# ============================================

# Lua (header-only interface)
set(LUA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/lua)
add_library(lua INTERFACE)
target_include_directories(lua INTERFACE ${LUA_INCLUDE_DIR})

# TinyXML2
add_subdirectory(third_party/tinyxml2 EXCLUDE_FROM_ALL)

# ============================================
# LuaUI Modules
# ============================================
add_subdirectory(src/luaui/rendering)

# ============================================
# LuaUI Library
# ============================================

# Collect source files when they exist
file(GLOB_RECURSE LUAUI_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE LUAUI_HEADERS
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.hpp
)

if(LUAUI_SOURCES)
    add_library(LuaUI STATIC ${LUAUI_SOURCES} ${LUAUI_HEADERS})

    target_include_directories(LuaUI
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/rendering/include>
            $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(LuaUI
        PUBLIC
            lua
            tinyxml2
            d2d1
            dwrite
            windowscodecs
            winmm
            ws2_32
    )

    target_compile_definitions(LuaUI PRIVATE LUAUI_BUILD)

    message(STATUS "LuaUI library created with ${LUAUI_SOURCES}")
else()
    message(STATUS "No source files found in src/, skipping LuaUI library creation")
    add_library(LuaUI INTERFACE)
endif()

# ============================================
# Examples
# ============================================
if(LUAUI_BUILD_EXAMPLES AND EXISTS ${CMAKE_SOURCE_DIR}/examples)
    add_subdirectory(examples)
endif()

# ============================================
# Tests
# ============================================
if(LUAUI_BUILD_TESTS AND EXISTS ${CMAKE_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================
# Configuration Summary
# ============================================
message(STATUS "")
message(STATUS "========== LuaUI Configuration ==========")
message(STATUS "Version:        ${PROJECT_VERSION}")
message(STATUS "Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Examples: ${LUAUI_BUILD_EXAMPLES}")
message(STATUS "Build Tests:    ${LUAUI_BUILD_TESTS}")
message(STATUS "Source Files:   ${LUAUI_SOURCES}")
message(STATUS "========================================")
message(STATUS "")
