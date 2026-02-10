# LuaUI XML Layout System

file(GLOB XML_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB XML_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_library(LuaUI_Xml STATIC ${XML_SOURCES} ${XML_HEADERS})

target_include_directories(LuaUI_Xml
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/third_party/tinyxml2>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
        ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
        ${CMAKE_SOURCE_DIR}/src/luaui/controls/include
        ${CMAKE_SOURCE_DIR}/src/luaui/controls/layout/include
)

target_link_libraries(LuaUI_Xml
    PUBLIC
        LuaUI_ControlsLayout
        tinyxml2
)

message(STATUS "LuaUI XML Layout module created")
