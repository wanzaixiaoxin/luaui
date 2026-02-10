cmake_minimum_required(VERSION 3.15)

# Enable testing
enable_testing()

# Common include directory for test framework
set(TEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Test executable for Controls Layout
if(TARGET LuaUI_ControlsLayout AND TARGET LuaUI)
    add_executable(test_layout test_layout.cpp)
    target_link_libraries(test_layout PRIVATE LuaUI LuaUI_ControlsLayout)
    target_include_directories(test_layout PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/controls/include
        ${CMAKE_SOURCE_DIR}/src/luaui/controls/layout/include
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
    )
    
    # Add test
    add_test(NAME LayoutTest COMMAND test_layout)
endif()

# Test executable for rendering types
if(TARGET LuaUI_Rendering)
    add_executable(test_rendering test_rendering.cpp)
    target_link_libraries(test_rendering PRIVATE LuaUI_Rendering)
    target_include_directories(test_rendering PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
        ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
    )

    # Add test
    add_test(NAME RenderingTest COMMAND test_rendering)
endif()

# Test executable for logger
if(TARGET LuaUI_Utils)
    add_executable(test_logger test_logger.cpp)
    target_link_libraries(test_logger PRIVATE LuaUI_Utils)
    target_include_directories(test_logger PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
    )

    # Add test
    add_test(NAME LoggerTest COMMAND test_logger)
endif()

# Test executable for style system
if(TARGET LuaUI_Style)
    add_executable(test_style_system test_style_system.cpp)
    target_link_libraries(test_style_system PRIVATE LuaUI_Style LuaUI)
    target_include_directories(test_style_system PRIVATE
        ${TEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/style/include
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
        ${CMAKE_SOURCE_DIR}/src/luaui/controls/include
    )

    # Add test
    add_test(NAME StyleSystemTest COMMAND test_style_system)
endif()
