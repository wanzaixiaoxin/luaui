add_executable(06_controls_demo WIN32 main.cpp)

target_link_libraries(06_controls_demo
    PRIVATE
        LuaUI
        d2d1
        dwrite
        windowscodecs
)

target_include_directories(06_controls_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/src/luaui/controls/include
    ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
    ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
)

set_target_properties(06_controls_demo PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
