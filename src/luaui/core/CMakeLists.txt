# LuaUI Core Module - Core infrastructure

add_library(LuaUI_Core STATIC
    Control.cpp
    Control.h
    Window.cpp
    Window.h
    Dispatcher.cpp
    Dispatcher.h
    Delegate.h
    Components/Component.cpp
    Components/Component.h
    Components/LayoutComponent.cpp
    Components/LayoutComponent.h
    Components/RenderComponent.cpp
    Components/RenderComponent.h
    Components/InputComponent.cpp
    Components/InputComponent.h
    Interfaces/IControl.h
    Interfaces/IRenderable.h
    Interfaces/ILayoutable.h
    Interfaces/IInputHandler.h
    Interfaces/INativeWindow.h
    Interfaces/Interfaces.h
    Interfaces/IStyleable.h
)

target_include_directories(LuaUI_Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
)

target_link_libraries(LuaUI_Core
    PUBLIC
        LuaUI_Rendering
        d2d1
        dwrite
        windowscodecs
)

target_compile_definitions(LuaUI_Core PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
