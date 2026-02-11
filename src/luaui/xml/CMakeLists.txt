# LuaUI XML Layout System

file(GLOB XML_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB XML_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(LuaUI_Xml STATIC ${XML_SOURCES} ${XML_HEADERS})

target_include_directories(LuaUI_Xml
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/third_party/tinyxml2>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
        ${CMAKE_SOURCE_DIR}/src/luaui/controls
        ${CMAKE_SOURCE_DIR}/src/luaui/core
)

target_link_libraries(LuaUI_Xml
    PUBLIC
        LuaUI_ControlsLayout
        LuaUI_Core
        tinyxml2
)

message(STATUS "LuaUI XML Layout module created")
