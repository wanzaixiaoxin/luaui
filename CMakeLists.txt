cmake_minimum_required(VERSION 3.15)

project(LuaUI
    VERSION 1.0.0
    DESCRIPTION "A commercial-grade Windows UI framework with Lua scripting"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_SAMPLES "Build sample applications" ON)
option(BUILD_TOOLS "Build development tools" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler options
if(MSVC)
    add_compile_options(/W4 /MP)
    # Disable specific warnings
    add_compile_options(/wd4819)  # Code page warning
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /GL /DNDEBUG")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find packages
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Third-party libraries
set(LUA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/lua")

# Check Lua headers
if(NOT EXISTS "${LUA_INCLUDE_DIR}/lua.h")
    message(WARNING "Lua headers not found in ${LUA_INCLUDE_DIR}")
endif()

# External dependencies
add_subdirectory(third_party/tinyxml2 EXCLUDE_FROM_ALL)

# Import Lua library
add_library(lua STATIC IMPORTED)
set_target_properties(lua PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libs/lua54.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
)

# Core modules
add_subdirectory(src/luaui/core)
add_subdirectory(src/luaui/utils)
add_subdirectory(src/luaui/platform)
add_subdirectory(src/luaui/rendering)
add_subdirectory(src/luaui/resources)
add_subdirectory(src/luaui/xml)
add_subdirectory(src/luaui/input)
add_subdirectory(src/luaui/binding)
add_subdirectory(src/luaui/styling)
add_subdirectory(src/luaui/animation)
add_subdirectory(src/luaui/layout)
add_subdirectory(src/luaui/ui)
add_subdirectory(src/luaui/scripting)

# LuaUI main interface library
add_library(LuaUI INTERFACE)
add_library(LuaUI::LuaUI ALIAS LuaUI)

target_link_libraries(LuaUI INTERFACE
    LuaUI_Core
    LuaUI_Utils
    LuaUI_Platform
    LuaUI_Rendering
    LuaUI_Resources
    LuaUI_XML
    LuaUI_Input
    LuaUI_Binding
    LuaUI_Styling
    LuaUI_Animation
    LuaUI_Layout
    LuaUI_UI
    LuaUI_Scripting
    lua
    tinyxml2
)

# Note: External dependencies (lua, tinyxml2) are not exported

target_include_directories(LuaUI INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/core/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/utils/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/platform/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/rendering/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/resources/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/xml/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/input/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/binding/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/styling/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/animation/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/layout/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/ui/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/scripting/include>
    $<INSTALL_INTERFACE:include>
)

# Samples
if(BUILD_SAMPLES)
    add_subdirectory(samples/01_hello_world)
    add_subdirectory(samples/02_notepad)
endif()

# Tools
if(BUILD_TOOLS)
    add_subdirectory(tools)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install configuration (disabled for development)
# TODO: Re-enable when install targets are properly configured
# include(GNUInstallDirs)
# install(EXPORT LuaUITargets
#     FILE LuaUITargets.cmake
#     NAMESPACE LuaUI::
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LuaUI
# )

# Print configuration summary
message(STATUS "")
message(STATUS "LuaUI Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build samples: ${BUILD_SAMPLES}")
message(STATUS "  Build tools: ${BUILD_TOOLS}")
message(STATUS "")
