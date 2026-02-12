cmake_minimum_required(VERSION 3.15)

# Windows header optimization
if(WIN32)
    add_compile_options(/DNOMINMAX /DWIN32_LEAN_AND_MEAN)
endif()

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
option(LUAUI_BUILD_OLD_EXAMPLES "Build old examples (06-12, not yet migrated)" OFF)

# Platform check
if(NOT WIN32)
    message(FATAL_ERROR "LuaUI currently only supports Windows")
endif()

# Compiler options for MSVC
if(MSVC)
    add_compile_options(/W4 /MP)
    add_compile_options(/wd4819)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(UNICODE _UNICODE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ============================================
# Third-party libraries
# ============================================
set(LUA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/lua)
add_library(lua INTERFACE)
target_include_directories(lua INTERFACE ${LUA_INCLUDE_DIR})

add_subdirectory(third_party/tinyxml2 EXCLUDE_FROM_ALL)

# ============================================
# LuaUI Modules
# ============================================
add_subdirectory(src/luaui/utils)
add_subdirectory(src/luaui/rendering)
add_subdirectory(src/luaui/core)
add_subdirectory(src/luaui/controls)
add_subdirectory(src/luaui/controls/layouts)  # Interface library
add_subdirectory(src/luaui/style)
add_subdirectory(src/luaui/xml)
add_subdirectory(src/luaui/mvvm)

# ============================================
# LuaUI Main Library (Interface)
# ============================================
add_library(LuaUI INTERFACE)

target_include_directories(LuaUI
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/core>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/controls>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/rendering>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/utils>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/style>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/xml>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/mvvm>

        $<INSTALL_INTERFACE:include>
)

target_link_libraries(LuaUI
    INTERFACE
        lua
        tinyxml2
        LuaUI_Core
        LuaUI_Controls
        LuaUI_Style
        LuaUI_Xml
        LuaUI_MVVM

        d2d1
        dwrite
        windowscodecs
        winmm
        ws2_32
)

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
message(STATUS "========================================")
message(STATUS "")
