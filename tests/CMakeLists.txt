cmake_minimum_required(VERSION 3.15)

# Enable testing
enable_testing()

# Common include directory for test framework
set(TEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Test executable for Controls Layout
if(TARGET LuaUI_Controls AND TARGET LuaUI)
    add_executable(test_layout test_layout.cpp)
    target_link_libraries(test_layout PRIVATE LuaUI LuaUI_Controls)
    target_include_directories(test_layout PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )
    add_test(NAME LayoutTest COMMAND test_layout)
endif()

# Test executable for rendering types
if(TARGET LuaUI_Rendering)
    add_executable(test_rendering test_rendering.cpp)
    target_link_libraries(test_rendering PRIVATE LuaUI_Rendering)
    target_include_directories(test_rendering PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )

    add_test(NAME RenderingTest COMMAND test_rendering)
endif()

# Test executable for rendering resources (DirtyRegion, ResourceCache)
if(TARGET LuaUI_Rendering)
    add_executable(test_rendering_resources test_rendering_resources.cpp)
    target_link_libraries(test_rendering_resources PRIVATE LuaUI_Rendering)
    target_include_directories(test_rendering_resources PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )

    add_test(NAME RenderingResourcesTest COMMAND test_rendering_resources)
endif()

# Test executable for core delegates
if(TARGET LuaUI_Core)
    add_executable(test_core_delegates test_core_delegates.cpp)
    target_link_libraries(test_core_delegates PRIVATE LuaUI_Core)
    target_include_directories(test_core_delegates PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )
    add_test(NAME CoreDelegatesTest COMMAND test_core_delegates)
endif()

# Test executable for core control
if(TARGET LuaUI_Core AND TARGET LuaUI_Controls)
    add_executable(test_core_control test_core_control.cpp)
    target_link_libraries(test_core_control PRIVATE LuaUI_Core LuaUI_Controls)
    target_include_directories(test_core_control PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )

    add_test(NAME CoreControlTest COMMAND test_core_control)
endif()

# Test executable for Grid layout
if(TARGET LuaUI_Controls)
    add_executable(test_layout_grid test_layout_grid.cpp)
    target_link_libraries(test_layout_grid PRIVATE LuaUI_Controls LuaUI_Core)
    target_include_directories(test_layout_grid PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )

    add_test(NAME GridLayoutTest COMMAND test_layout_grid)
endif()

# Test executable for Lua binding
if(TARGET LuaUI_Lua)
    add_executable(test_lua_binding test_lua_binding.cpp)
    target_link_libraries(test_lua_binding PRIVATE LuaUI_Lua LuaUI_Core LuaUI_Controls)
    target_include_directories(test_lua_binding PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/lua
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/third_party/lua
    )

    add_test(NAME LuaBindingTest COMMAND test_lua_binding)
endif()

# Test executable for logger
if(TARGET LuaUI_Utils)
    add_executable(test_logger test_logger.cpp)
    target_link_libraries(test_logger PRIVATE LuaUI_Utils)
    target_include_directories(test_logger PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )

    add_test(NAME LoggerTest COMMAND test_logger)
endif()

# Test executable for style system
if(TARGET LuaUI_Style)
    add_executable(test_style_system test_style_system.cpp)
    target_link_libraries(test_style_system PRIVATE LuaUI_Style LuaUI_Core LuaUI_Controls LuaUI_Rendering)
    target_include_directories(test_style_system PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/style
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )
    add_test(NAME StyleSystemTest COMMAND test_style_system)
endif()

# Test executable for MVVM binding
if(TARGET LuaUI_MVVM)
    add_executable(test_mvvm_binding test_mvvm_binding.cpp)
    target_link_libraries(test_mvvm_binding PRIVATE LuaUI_MVVM LuaUI_Core)
    target_include_directories(test_mvvm_binding PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/mvvm
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )
    add_test(NAME MvvmBindingTest COMMAND test_mvvm_binding)
endif()

# Test executable for extended controls (Slider, ProgressBar, TextBox, Image)
if(TARGET LuaUI_Controls)
    add_executable(test_controls_extended test_controls_extended.cpp)
    target_link_libraries(test_controls_extended PRIVATE LuaUI_Controls LuaUI_Core LuaUI_Rendering)
    target_include_directories(test_controls_extended PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )
    add_test(NAME ControlsExtendedTest COMMAND test_controls_extended)
endif()

# Test executable for event system (Delegate)
if(TARGET LuaUI_Core AND TARGET LuaUI_Controls)
    add_executable(test_events test_events.cpp)
    target_link_libraries(test_events PRIVATE LuaUI_Core LuaUI_Controls)
    target_include_directories(test_events PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )
    add_test(NAME EventsTest COMMAND test_events)
endif()

# Test executable for TextBox editing behaviour
if(TARGET LuaUI_Core AND TARGET LuaUI_Controls)
    add_executable(test_textbox_editing test_textbox_editing.cpp)
    target_link_libraries(test_textbox_editing PRIVATE LuaUI_Core LuaUI_Controls LuaUI_Rendering)
    target_include_directories(test_textbox_editing PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )
    add_test(NAME TextBoxEditingTest COMMAND test_textbox_editing)
endif()

# Test executable for XML integration (XML layout loading, MVVM binding, XML+Lua integration)
if(TARGET LuaUI_Xml AND TARGET LuaUI_MVVM AND TARGET LuaUI_Lua)
    add_executable(test_xml_integration test_xml_integration.cpp)
    target_link_libraries(test_xml_integration PRIVATE 
        LuaUI_Xml 
        LuaUI_MVVM 
        LuaUI_Lua 
        LuaUI_Core 
        LuaUI_Controls
    )
    target_include_directories(test_xml_integration PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/xml
        ${CMAKE_SOURCE_DIR}/src/luaui/mvvm
        ${CMAKE_SOURCE_DIR}/src/luaui/lua
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/third_party/lua
    )
    add_test(NAME XmlIntegrationTest COMMAND test_xml_integration)
endif()

# Test executable for E2E integration (complete XML+Lua+MVVM pipeline)
if(TARGET LuaUI_Xml AND TARGET LuaUI_MVVM AND TARGET LuaUI_Lua)
    add_executable(test_e2e_integration test_e2e_integration.cpp)
    target_link_libraries(test_e2e_integration PRIVATE 
        LuaUI_Xml 
        LuaUI_MVVM 
        LuaUI_Lua 
        LuaUI_Core 
        LuaUI_Controls
    )
    target_include_directories(test_e2e_integration PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/xml
        ${CMAKE_SOURCE_DIR}/src/luaui/mvvm
        ${CMAKE_SOURCE_DIR}/src/luaui/lua
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/third_party/lua
    )
    add_test(NAME E2EIntegrationTest COMMAND test_e2e_integration)
endif()

# Visual Tests with Lua MVVM
add_subdirectory(visual)


