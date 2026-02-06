# UI Module - UI控件系统

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_UI)

set(HEADERS
    include/luaui/ui/BaseControl.h
    include/luaui/ui/ButtonControl.h
    include/luaui/ui/EditControl.h
    include/luaui/ui/GenericControl.h
    include/luaui/ui/LabelControl.h
    include/luaui/ui/WindowControl.h
    include/luaui/ui/EventManager.h
    include/luaui/ui/EventRouter.h
    include/luaui/ui/LuaEventHandler.h
    include/luaui/ui/ControlFactory.h
    include/luaui/ui/ContainerLayout.h
    include/luaui/ui/LayoutEngine.h
    include/luaui/ui/PositionCalculator.h
)

set(SOURCES
    src/elements/BaseControl.cpp
    src/controls/ButtonControl.cpp
    src/controls/GenericControl.cpp
    src/controls/LabelControl.cpp
    src/controls/WindowControl.cpp
    src/EventManager.cpp
    src/EventRouter.cpp
    src/LuaEventHandler.cpp
    src/ControlFactory.cpp
    src/ContainerLayout.cpp
    src/LayoutEngine.cpp
    src/PositionCalculator.cpp
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
    LuaUI_Resources
)

install(TARGETS ${MODULE_NAME}
    EXPORT LuaUITargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
