# XML module

file(GLOB_RECURSE XML_SOURCES "*.cpp")

add_library(LuaUI_XML STATIC ${XML_SOURCES})

target_include_directories(LuaUI_XML PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/tinyxml2
)

target_link_libraries(LuaUI_XML
    tinyxml2
    LuaUI_Core
)
