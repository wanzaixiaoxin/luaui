# Resources Module - 资源管理

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_Resources)

set(HEADERS
    include/luaui/resources/ImageLoader.h
    include/luaui/resources/StyleManager.h
    include/luaui/resources/ThemeManager.h
)

set(SOURCES
    src/ImageLoader.cpp
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

install(TARGETS ${MODULE_NAME}
    EXPORT LuaUITargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
