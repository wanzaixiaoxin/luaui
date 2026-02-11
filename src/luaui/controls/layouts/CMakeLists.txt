# LuaUI Controls Layout - Interface library for backward compatibility

add_library(LuaUI_ControlsLayout INTERFACE)

target_include_directories(LuaUI_ControlsLayout
    INTERFACE
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
)

target_link_libraries(LuaUI_ControlsLayout
    INTERFACE
        LuaUI_Controls
)
