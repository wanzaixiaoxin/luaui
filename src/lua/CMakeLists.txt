# Lua module

file(GLOB_RECURSE LUA_SOURCES "*.cpp")

add_library(LuaUI_Lua STATIC ${LUA_SOURCES})

target_include_directories(LuaUI_Lua PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/lua
)

target_link_libraries(LuaUI_Lua
    lua
    LuaUI_Core
    LuaUI_Utils
)
