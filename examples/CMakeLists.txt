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
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )
endif()

# 04_animation_demo - Animation system demo
if(TARGET LuaUI_Rendering)
    add_executable(04_animation_demo WIN32 04_animation_demo.cpp)
    target_link_libraries(04_animation_demo PRIVATE LuaUI_Rendering)
    target_include_directories(04_animation_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )
endif()

# 05_offscreen_demo - Offscreen rendering demo
if(TARGET LuaUI_Rendering)
    add_executable(05_offscreen_demo WIN32 05_offscreen_demo.cpp)
    target_link_libraries(05_offscreen_demo PRIVATE LuaUI_Rendering)
    target_include_directories(05_offscreen_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
    )
endif()

# 06_controls_demo - Control system demo (New Architecture)
if(TARGET LuaUI)
    # 使用控制台子系统以便查看日志
    add_executable(06_controls_demo 06_controls_demo/main.cpp)
    target_link_libraries(06_controls_demo PRIVATE LuaUI)
    target_include_directories(06_controls_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
    )
endif()

# 07_layout_demo - Layout engine demo (New Architecture)
if(TARGET LuaUI)
    add_executable(07_layout_demo 07_layout_demo/main.cpp)
    target_link_libraries(07_layout_demo PRIVATE LuaUI)
    target_include_directories(07_layout_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
    )
endif()

# 08_visual_layout_demo - Visual layout demo with GUI (New Architecture)
if(TARGET LuaUI)
    add_executable(08_visual_layout_demo WIN32 08_visual_layout_demo/main.cpp)
    target_link_libraries(08_visual_layout_demo PRIVATE LuaUI)
    target_include_directories(08_visual_layout_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
    )
endif()

# 09_composite_layout - Layout + Controls composite demo (New Architecture)
if(TARGET LuaUI)
    add_executable(09_composite_layout WIN32 09_composite_layout/main.cpp)
    target_link_libraries(09_composite_layout PRIVATE LuaUI)
    target_include_directories(09_composite_layout PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
    )
endif()

# 10_advanced_controls - Advanced input controls demo (New Architecture)
if(TARGET LuaUI)
    add_executable(10_advanced_controls WIN32 10_advanced_controls/main.cpp)
    target_link_libraries(10_advanced_controls PRIVATE LuaUI)
    target_include_directories(10_advanced_controls PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
    )
endif()

# 11_style_demo - Style system demo (New Architecture)
if(TARGET LuaUI)
    add_executable(11_style_demo WIN32 11_style_demo/main.cpp)
    target_link_libraries(11_style_demo PRIVATE LuaUI)
    target_include_directories(11_style_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/style
    )
endif()

# 12_xml_layout_demo - XML layout system demo with code-behind (New Architecture)
if(TARGET LuaUI)
    add_executable(12_xml_layout_demo WIN32 
        12_xml_layout_demo/main.cpp
        12_xml_layout_demo/MainWindow.cpp
        12_xml_layout_demo/MainWindow.h
    )
    target_link_libraries(12_xml_layout_demo PRIVATE LuaUI)
    target_include_directories(12_xml_layout_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/style
        ${CMAKE_SOURCE_DIR}/src/luaui/xml
        ${CMAKE_SOURCE_DIR}/third_party/tinyxml2
        12_xml_layout_demo
    )
endif()

# 13_refactored_arch_demo - 重构后的架构演示
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
