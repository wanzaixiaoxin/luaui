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

# 06_controls_demo - Control system demo
if(TARGET LuaUI)
    add_subdirectory(06_controls_demo)
endif()

# Copy resources to output directory
if(EXISTS ${CMAKE_SOURCE_DIR}/resources)
    file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
