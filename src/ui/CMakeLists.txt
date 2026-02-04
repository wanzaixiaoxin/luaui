# UI module

file(GLOB_RECURSE UI_SOURCES "*.cpp")

add_library(LuaUI_UI STATIC ${UI_SOURCES})

target_include_directories(LuaUI_UI PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LuaUI_UI
    LuaUI_Core
    LuaUI_XML
    lua
)
