# Scripting Module - Lua脚本系统

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_Scripting)

set(HEADERS
    include/luaui/scripting/ControlBinder.h
    include/luaui/scripting/EventBinder.h
    include/luaui/scripting/LoggerBinder.h
    include/luaui/scripting/LuaBinder.h
    include/luaui/scripting/LifecycleManager.h
    include/luaui/scripting/ScriptLoader.h
    include/luaui/scripting/ScriptRunner.h
)

set(SOURCES
    src/bindings/ControlBinder.cpp
    src/bindings/EventBinder.cpp
    src/bindings/LoggerBinder.cpp
    src/bindings/LuaBinder.cpp
    src/LifecycleManager.cpp
    src/ScriptLoader.cpp
    src/ScriptRunner.cpp
)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${LUA_INCLUDE_DIR}
)

target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)

# Dependencies
target_link_libraries(${MODULE_NAME} PUBLIC
    LuaUI_Core
    LuaUI_Utils
)

target_link_libraries(${MODULE_NAME} PRIVATE
    lua
)

install(TARGETS ${MODULE_NAME}
    EXPORT LuaUITargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
