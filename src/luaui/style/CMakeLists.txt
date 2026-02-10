# LuaUI Style System CMakeLists.txt

cmake_minimum_required(VERSION 3.15)

# 样式系统库
set(STYLE_SOURCES
    src/ResourceDictionary.cpp
    src/Setter.cpp
    src/Trigger.cpp
    src/Style.cpp
    src/Theme.cpp
)

set(STYLE_HEADERS
    include/ResourceDictionary.h
    include/Setter.h
    include/Trigger.h
    include/Style.h
    include/Theme.h
)

# 创建样式系统库
add_library(LuaUI_Style STATIC ${STYLE_SOURCES} ${STYLE_HEADERS})

# 设置包含目录
target_include_directories(LuaUI_Style
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
        ${CMAKE_SOURCE_DIR}/src/luaui/controls/include
        ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
)

# 链接依赖
target_link_libraries(LuaUI_Style
    PUBLIC
        LuaUI_Rendering
        LuaUI_Utils
)

# C++ 标准
set_target_properties(LuaUI_Style PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 分组
source_group("Headers" FILES ${STYLE_HEADERS})
source_group("Sources" FILES ${STYLE_SOURCES})
