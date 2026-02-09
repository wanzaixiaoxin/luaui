add_executable(08_visual_layout_demo WIN32 main.cpp)

target_link_libraries(08_visual_layout_demo
    LuaUI
    LuaUI_ControlsLayout
)

target_include_directories(08_visual_layout_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/src/luaui/controls/include
    ${CMAKE_SOURCE_DIR}/src/luaui/controls/layout/include
    ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
    ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
)

set_target_properties(08_visual_layout_demo PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
