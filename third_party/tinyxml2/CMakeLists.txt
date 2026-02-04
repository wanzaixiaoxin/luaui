# TinyXML2 Library CMakeLists.txt
cmake_minimum_required(VERSION 3.15)

# TinyXML2源文件
set(TINYXML2_SRC
    tinyxml2.cpp
)

# 创建静态库
add_library(tinyxml2 STATIC ${TINYXML2_SRC})

# 设置包含目录
target_include_directories(tinyxml2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 编译定义
target_compile_definitions(tinyxml2 PRIVATE 
    TINYXML2_EXPORT
    _CRT_SECURE_NO_WARNINGS
)

# C++编译器设置
set_target_properties(tinyxml2 PROPERTIES 
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(tinyxml2 PRIVATE /W4)
else()
    target_compile_options(tinyxml2 PRIVATE -Wall -Wextra -pedantic)
endif()
