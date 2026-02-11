cmake_minimum_required(VERSION 3.15)

# 01_basic_window - Win32 + Direct2D
add_executable(01_basic_window WIN32 01_basic_window.cpp)
target_link_libraries(01_basic_window PRIVATE d2d1)

# 02_hello_lua - Lua integration
add_executable(02_hello_lua 02_hello_lua.cpp)
target_include_directories(02_hello_lua PRIVATE ${CMAKE_SOURCE_DIR}/third_party)

# 03_rendering_demo - Full rendering engine demo
if(TARGET LuaUI_Rendering)
    add_executable(03_rendering_demo WIN32 03_rendering_demo.cpp)
    target_link_libraries(03_rendering_demo PRIVATE LuaUI_Rendering)
    target_include_directories(03_rendering_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
        ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
    )
endif()

# 04_animation_demo - Animation system demo
if(TARGET LuaUI_Rendering)
    add_executable(04_animation_demo WIN32 04_animation_demo.cpp)
    target_link_libraries(04_animation_demo PRIVATE LuaUI_Rendering)
    target_include_directories(04_animation_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
        ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
    )
endif()

# 05_offscreen_demo - Offscreen rendering demo
if(TARGET LuaUI_Rendering)
    add_executable(05_offscreen_demo WIN32 05_offscreen_demo.cpp)
    target_link_libraries(05_offscreen_demo PRIVATE LuaUI_Rendering)
    target_include_directories(05_offscreen_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
        ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
    )
endif()

# TODO: Examples 06-12 are disabled until fully migrated to new architecture
# # 06_controls_demo - Control system demo
# if(TARGET LuaUI)
#     add_subdirectory(06_controls_demo)
# endif()
# 
# # 07_layout_demo - Layout engine demo
# if(TARGET LuaUI_ControlsLayout)
#     add_subdirectory(07_layout_demo)
# endif()
# 
# # 08_visual_layout_demo - Visual layout demo with GUI
# if(TARGET LuaUI_ControlsLayout)
#     add_subdirectory(08_visual_layout_demo)
# endif()
# 
# # 09_composite_layout - Layout + Controls composite demo
# if(TARGET LuaUI_ControlsLayout)
#     add_subdirectory(09_composite_layout)
# endif()
# 
# # 10_advanced_controls - Advanced input controls demo
# if(TARGET LuaUI)
#     add_subdirectory(10_advanced_controls)
# endif()
# 
# # 11_style_demo - Style system demo
# if(TARGET LuaUI_Style)
#     add_subdirectory(11_style_demo)
# endif()
# 
# # 12_xml_layout_demo - XML layout system demo with code-behind
# if(TARGET LuaUI_Xml AND TARGET LuaUI_Core)
#     add_executable(12_xml_layout_demo WIN32 
#         12_xml_layout_demo/main.cpp
#         12_xml_layout_demo/MainWindow.cpp
#         12_xml_layout_demo/MainWindow.h
#     )
#     target_link_libraries(12_xml_layout_demo PRIVATE LuaUI LuaUI_Xml LuaUI_Core)
#     target_include_directories(12_xml_layout_demo PRIVATE
#         ${CMAKE_SOURCE_DIR}/src
#         ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
#         ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
#         ${CMAKE_SOURCE_DIR}/src/luaui/controls/include
#         ${CMAKE_SOURCE_DIR}/src/luaui/controls/layout/include
#         ${CMAKE_SOURCE_DIR}/src/luaui/xml/include
#         ${CMAKE_SOURCE_DIR}/src/luaui/core/include
#         ${CMAKE_SOURCE_DIR}/third_party/tinyxml2
#         12_xml_layout_demo
#     )
# endif()

# 13_refactored_arch_demo - SOLID重构后的架构演示
if(TARGET LuaUI_Core)
    add_executable(13_refactored_arch_demo 
        13_refactored_arch_demo/main.cpp
        13_refactored_arch_demo/MainWindow.cpp
        13_refactored_arch_demo/MainWindow.h
    )
    target_link_libraries(13_refactored_arch_demo PRIVATE LuaUI)
    target_include_directories(13_refactored_arch_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        13_refactored_arch_demo
    )
endif()

# Copy resources to output directory
if(EXISTS ${CMAKE_SOURCE_DIR}/resources)
    file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
