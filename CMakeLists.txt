cmake_minimum_required(VERSION 3.15)
project(LuaUI VERSION 1.0.0 LANGUAGES CXX)

# 设置C++11标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /MT)
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /GL /DNDEBUG")
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找第三方库
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Lua使用DLL方式，头文件在third_party/lua
set(LUA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/lua")
set(LUA_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/libs")

# 检查Lua头文件
if(NOT EXISTS "${LUA_INCLUDE_DIR}/lua.h")
    message(FATAL_ERROR "Lua headers not found in ${LUA_INCLUDE_DIR}. Please place Lua headers there.")
endif()

# 查找Lua库文件
find_library(LUA_LIBRARY 
    NAMES lua54 lua5.4 lua liblua
    PATHS ${LUA_LIBRARY_DIR}
    NO_DEFAULT_PATH
)
if(NOT LUA_LIBRARY)
    message(WARNING "Lua library not found in ${LUA_LIBRARY_DIR}. Please place lua54.dll/lib there.")
endif()

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${LUA_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/tinyxml2
)

# 第三方库 - TinyXML2（源码编译）
add_subdirectory(third_party/tinyxml2 EXCLUDE_FROM_ALL)

# 创建Lua导入库目标（如果找到库文件）
if(LUA_LIBRARY)
    add_library(lua SHARED IMPORTED GLOBAL)
    set_target_properties(lua PROPERTIES
        IMPORTED_LOCATION "${LUA_LIBRARY_DIR}/lua54.dll"
        IMPORTED_IMPLIB "${LUA_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
    )
else()
    # 如果没有找到库文件，创建一个接口库，只提供头文件路径
    add_library(lua INTERFACE IMPORTED GLOBAL)
    set_target_properties(lua PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
    )
endif()

# 核心库
add_subdirectory(src/core)

# XML模块
add_subdirectory(src/xml)

# UI模块
add_subdirectory(src/ui)

# Lua模块
add_subdirectory(src/lua)

# 资源模块
add_subdirectory(src/resource)

# 工具模块
add_subdirectory(src/utils)

# 创建统一的LuaUI接口库
add_library(LuaUI INTERFACE)
target_link_libraries(LuaUI INTERFACE
    LuaUI_Core
    LuaUI_XML
    LuaUI_UI
    LuaUI_Lua
    LuaUI_Resource
    LuaUI_Utils
    lua
    tinyxml2
)

# 示例应用
add_subdirectory(examples/hello_world)
add_subdirectory(examples/notepad)

# 安装配置
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY docs/ DESTINATION docs/share/luaui)

# 安装所有库目标
install(TARGETS 
    LuaUI LuaUI_Core LuaUI_XML LuaUI_UI LuaUI_Lua LuaUI_Resource LuaUI_Utils
    tinyxml2
    DESTINATION lib
)

# 安装Lua DLL（如果存在）
if(EXISTS "${LUA_LIBRARY_DIR}/lua54.dll")
    install(FILES "${LUA_LIBRARY_DIR}/lua54.dll" DESTINATION bin)
endif()
