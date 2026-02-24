cmake_minimum_required(VERSION 3.15)

# 01_controls_demo - Control system demo (Code-based)
if(TARGET LuaUI)
    add_executable(01_controls_demo 01_controls_demo/main.cpp)
    target_link_libraries(01_controls_demo PRIVATE LuaUI)
    target_include_directories(01_controls_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
    )
endif()

# 02_mvvm_demo - MVVM data binding demo (XML-based)
if(TARGET LuaUI)
    add_executable(02_mvvm_demo
        02_mvvm_demo/main.cpp
        02_mvvm_demo/MainWindow.cpp
        02_mvvm_demo/MainWindow.h
    )
    target_link_libraries(02_mvvm_demo PRIVATE LuaUI)
    target_include_directories(02_mvvm_demo PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/style
        ${CMAKE_SOURCE_DIR}/src/luaui/xml
        ${CMAKE_SOURCE_DIR}/src/luaui/mvvm
        ${CMAKE_SOURCE_DIR}/third_party/tinyxml2
        02_mvvm_demo
    )
endif()

# Copy 02_mvvm_demo layouts to output directory
if(TARGET 02_mvvm_demo AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/02_mvvm_demo/mvvm_layouts)
    add_custom_command(TARGET 02_mvvm_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/02_mvvm_demo/mvvm_layouts
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mvvm_layouts
        COMMENT "Copying MVVM layouts to output directory"
    )
endif()

# 03_lua_mvvm_demo - MVVM with Lua ViewModel (Phase 10)
# Demonstrates: XML (View) + Lua (ViewModel) architecture
if(TARGET LuaUI AND TARGET LuaUI_Lua)
    add_subdirectory(03_lua_mvvm_demo)
endif()
