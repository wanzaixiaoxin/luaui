# LuaUI Style System CMakeLists.txt

cmake_minimum_required(VERSION 3.15)

set(STYLE_SOURCES
    ResourceDictionary.cpp
    Setter.cpp
    Trigger.cpp
    Style.cpp
    Theme.cpp
)

set(STYLE_HEADERS
    ResourceDictionary.h
    Setter.h
    Trigger.h
    Style.h
    Theme.h
)

add_library(LuaUI_Style STATIC ${STYLE_SOURCES} ${STYLE_HEADERS})

target_include_directories(LuaUI_Style
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
)

target_link_libraries(LuaUI_Style
    PUBLIC
        LuaUI_Core
        LuaUI_Rendering
        LuaUI_Utils
)

set_target_properties(LuaUI_Style PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

source_group("Headers" FILES ${STYLE_HEADERS})
source_group("Sources" FILES ${STYLE_SOURCES})
