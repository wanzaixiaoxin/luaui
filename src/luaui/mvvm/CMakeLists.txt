# MVVM 数据绑定模块

cmake_minimum_required(VERSION 3.15)

# MVVM 库
add_library(LuaUI_MVVM STATIC
    BindingEngine.cpp
    XmlBindingExtension.cpp
    MvvmXmlLoader.cpp
    ViewModelBase.cpp
)

target_include_directories(LuaUI_MVVM
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/third_party/tinyxml2
)

target_link_libraries(LuaUI_MVVM
    PUBLIC
        LuaUI_Core
        LuaUI_Controls
        LuaUI_Utils
        LuaUI_Xml
)

# 设置C++标准
set_target_properties(LuaUI_MVVM PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
