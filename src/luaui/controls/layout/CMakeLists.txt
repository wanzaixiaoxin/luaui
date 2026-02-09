# LuaUI Controls Layout
# Layout containers for the Controls system

cmake_minimum_required(VERSION 3.15)

set(LAYOUT_SOURCES
    src/LayoutPanel.cpp
    src/StackPanel.cpp
    src/Grid.cpp
    src/Canvas.cpp
    src/DockPanel.cpp
    src/WrapPanel.cpp
)

set(LAYOUT_HEADERS
    include/layout.h
    include/LayoutPanel.h
    include/StackPanel.h
    include/Grid.h
    include/Canvas.h
    include/DockPanel.h
    include/WrapPanel.h
)

add_library(LuaUI_ControlsLayout STATIC ${LAYOUT_SOURCES} ${LAYOUT_HEADERS})

target_include_directories(LuaUI_ControlsLayout
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/controls/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/rendering/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/utils/include>
)

target_compile_features(LuaUI_ControlsLayout PUBLIC cxx_std_17)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LAYOUT_SOURCES} ${LAYOUT_HEADERS})
