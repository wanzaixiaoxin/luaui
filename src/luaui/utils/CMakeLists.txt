# Utils Module - 工具库

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_Utils)

set(HEADERS
    include/luaui/utils/Logger.h
)

set(SOURCES
    src/EncodingUtils.cpp
    src/FileUtils.cpp
    src/Logger.cpp
    src/NumberUtils.cpp
    src/StringUtils.cpp
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
)

install(TARGETS ${MODULE_NAME}
    EXPORT LuaUITargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
