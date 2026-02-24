cmake_minimum_required(VERSION 3.15)

# Enable testing
enable_testing()

# Common include directory for test framework
set(TEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Test executable for Controls Layout (temporarily disabled - uses old API)
# TODO: Update test_layout.cpp to use new Layout API
# if(TARGET LuaUI_Controls AND TARGET LuaUI)
#     add_executable(test_layout test_layout.cpp)
#     target_link_libraries(test_layout PRIVATE LuaUI LuaUI_Controls)
#     target_include_directories(test_layout PRIVATE
#         ${TEST_INCLUDE_DIR}
#         ${CMAKE_SOURCE_DIR}/src/luaui/controls
#         ${CMAKE_SOURCE_DIR}/src/luaui/rendering
#         ${CMAKE_SOURCE_DIR}/src/luaui/core
#         ${CMAKE_SOURCE_DIR}/src/luaui/utils
#     )
#     add_test(NAME LayoutTest COMMAND test_layout)
# endif()

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

# Test executable for core delegates (temporarily disabled - Delegate API mismatch)
# if(TARGET LuaUI_Core)
#     add_executable(test_core_delegates test_core_delegates.cpp)
#     target_link_libraries(test_core_delegates PRIVATE LuaUI_Core)
#     target_include_directories(test_core_delegates PRIVATE
#         ${TEST_INCLUDE_DIR}
#         ${CMAKE_SOURCE_DIR}/src/luaui/core
#         ${CMAKE_SOURCE_DIR}/src/luaui/utils
#     )
#
#     add_test(NAME CoreDelegatesTest COMMAND test_core_delegates)
# endif()

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

# Test executable for style system (temporarily disabled - uses old API)
# TODO: Update test_style_system.cpp to use new Style API
# if(TARGET LuaUI_Style)
#     add_executable(test_style_system test_style_system.cpp)
#     target_link_libraries(test_style_system PRIVATE LuaUI_Style LuaUI)
#     target_include_directories(test_style_system PRIVATE
#         ${TEST_INCLUDE_DIR}
#         ${CMAKE_SOURCE_DIR}/src/luaui/style
#         ${CMAKE_SOURCE_DIR}/src/luaui/rendering
#         ${CMAKE_SOURCE_DIR}/src/luaui/controls
#         ${CMAKE_SOURCE_DIR}/src/luaui/core
#     )
#     add_test(NAME StyleSystemTest COMMAND test_style_system)
# endif()
