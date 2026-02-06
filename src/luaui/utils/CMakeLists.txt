# Utils module - Logging and utilities

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_Utils)

# Source files
set(SOURCES
    src/Logger.cpp
)

set(HEADERS
    include/Logger.h
)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

set_target_properties(${MODULE_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)

# Link with threading library
find_package(Threads REQUIRED)
target_link_libraries(${MODULE_NAME} PUBLIC Threads::Threads)

install(TARGETS ${MODULE_NAME}
    EXPORT LuaUITargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS} DESTINATION include)
