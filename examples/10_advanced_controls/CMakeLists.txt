add_executable(10_advanced_controls WIN32 main.cpp)

target_link_libraries(10_advanced_controls PRIVATE
    LuaUI
    LuaUI_ControlsLayout
)

set_target_properties(10_advanced_controls PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
