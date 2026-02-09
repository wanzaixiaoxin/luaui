# LuaUI Core - Base types and UIElement

cmake_minimum_required(VERSION 3.15)

set(CORE_SOURCES
    src/UIElement.cpp
    src/Panel.cpp
)

set(CORE_HEADERS
    include/luaui/Types.h
    include/luaui/UIElement.h
    include/luaui/Panel.h
)

add_library(LuaUI_Core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(LuaUI_Core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(LuaUI_Core PUBLIC cxx_std_17)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CORE_SOURCES} ${CORE_HEADERS})
