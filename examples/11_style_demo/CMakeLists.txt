# 11_style_demo - Style System Demo

set(SOURCES main.cpp)

add_executable(11_style_demo ${SOURCES})

set_target_properties(11_style_demo PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(11_style_demo PRIVATE
    LuaUI
    LuaUI_Style
)

if(WIN32)
    set_property(TARGET 11_style_demo PROPERTY WIN32_EXECUTABLE TRUE)
endif()
