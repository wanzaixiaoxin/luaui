add_executable(09_composite_layout WIN32 main.cpp)

target_link_libraries(09_composite_layout
    LuaUI
    LuaUI_ControlsLayout
)

target_include_directories(09_composite_layout PRIVATE
    ${CMAKE_SOURCE_DIR}/src/luaui/controls/include
    ${CMAKE_SOURCE_DIR}/src/luaui/controls/layout/include
    ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
    ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
)

set_target_properties(09_composite_layout PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
