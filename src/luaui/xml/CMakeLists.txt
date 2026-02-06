# XML Module - XML解析

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_XML)

set(HEADERS
    include/luaui/xml/LayoutValidator.h
    include/luaui/xml/ResourceParser.h
    include/luaui/xml/XmlParser.h
)

set(SOURCES
    src/LayoutValidator.cpp
    src/ResourceParser.cpp
    src/XmlParser.cpp
)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)

# Dependencies
target_link_libraries(${MODULE_NAME} PUBLIC
    LuaUI_Core
    LuaUI_Utils
)

target_link_libraries(${MODULE_NAME} PRIVATE
    tinyxml2
)

install(TARGETS ${MODULE_NAME}
    EXPORT LuaUITargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
