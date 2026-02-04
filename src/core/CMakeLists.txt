# Core module

file(GLOB_RECURSE CORE_SOURCES "*.cpp")

add_library(LuaUI_Core STATIC ${CORE_SOURCES})

target_include_directories(LuaUI_Core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/lua
    ${CMAKE_SOURCE_DIR}/third_party/tinyxml2
)

target_link_libraries(LuaUI_Core
    lua
    tinyxml2
)
