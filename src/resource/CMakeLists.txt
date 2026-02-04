# Resource module

file(GLOB_RECURSE RESOURCE_SOURCES "*.cpp")

add_library(LuaUI_Resource STATIC ${RESOURCE_SOURCES})

target_include_directories(LuaUI_Resource PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LuaUI_Resource
    LuaUI_Core
    tinyxml2
)
