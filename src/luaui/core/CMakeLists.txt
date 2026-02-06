# Core Module - 基础核心

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_Core)

set(HEADERS
    include/luaui/core/App.h
    include/luaui/core/LuaState.h
    include/luaui/core/ScriptEngine.h
    include/luaui/core/Utils.h
    # Public headers
    include/luaui/LuaUI.h
    include/luaui/ILuaUI.h
    include/luaui/Version.h
)

set(SOURCES
    src/App.cpp
    src/LuaState.cpp
    src/LuaUI.cpp
    src/ScriptEngine.cpp
    src/Utils.cpp
)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)

# Windows specific
if(WIN32)
    target_compile_definitions(${MODULE_NAME} PRIVATE
        _WINDOWS
        LUAUI_EXPORTS
    )
endif()

# Install
install(TARGETS ${MODULE_NAME}
    EXPORT LuaUITargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

# Tests
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
