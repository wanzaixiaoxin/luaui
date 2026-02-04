# Utils module

file(GLOB_RECURSE UTILS_SOURCES "*.cpp")

add_library(LuaUI_Utils STATIC ${UTILS_SOURCES})

target_include_directories(LuaUI_Utils PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LuaUI_Utils
    LuaUI_Core
)
