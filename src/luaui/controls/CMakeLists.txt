# LuaUI Controls Module

add_library(LuaUI_Controls STATIC
    Border.cpp
    Border.h
    Button.cpp
    Button.h
    CheckBox.cpp
    CheckBox.h
    Image.cpp
    Image.h
    ListBox.cpp
    ListBox.h
    Panel.cpp
    Panel.h
    Shapes.cpp
    Shapes.h
    Slider.cpp
    Slider.h
    TextBlock.cpp
    TextBlock.h
    TextBox.cpp
    TextBox.h
    Controls.h
    # Layouts
    layouts/StackPanel.cpp
    layouts/Grid.cpp
    layouts/Grid.h
    layouts/Canvas.cpp
    layouts/Canvas.h
    layouts/DockPanel.cpp
    layouts/DockPanel.h
    layouts/WrapPanel.cpp
    layouts/WrapPanel.h
    layouts/ScrollViewer.cpp
    layouts/ScrollViewer.h
    layouts/Viewbox.cpp
    layouts/Viewbox.h
    layouts/Layouts.h
    layouts/layout.h
)

target_include_directories(LuaUI_Controls
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/luaui/core
        ${CMAKE_SOURCE_DIR}/src/luaui/rendering
        ${CMAKE_SOURCE_DIR}/src/luaui/utils
)

target_link_libraries(LuaUI_Controls
    PUBLIC
        LuaUI_Core
)

target_compile_features(LuaUI_Controls PUBLIC cxx_std_17)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
