# LuaUI Core Module - Window and application framework

add_library(LuaUI_Core STATIC
    src/Window.cpp
)

target_include_directories(LuaUI_Core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/controls/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/controls/layout/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/rendering/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/luaui/utils/include>
)

target_link_libraries(LuaUI_Core
    PUBLIC
        LuaUI_Rendering
        LuaUI_ControlsLayout
        d2d1
        dwrite
        windowscodecs
)

# Add NOMINMAX and WIN32_LEAN_AND_MEAN for Windows headers
target_compile_definitions(LuaUI_Core PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
