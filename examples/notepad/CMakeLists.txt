# Notepad example

file(GLOB NOTEPAD_SOURCES "*.cpp")

add_executable(Notepad ${NOTEPAD_SOURCES})

target_include_directories(Notepad PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(Notepad
    LuaUI_Core
    LuaUI_XML
    LuaUI_UI
    LuaUI_Lua
    LuaUI_Resource
    LuaUI_Utils
)

# Copy Lua and XML files to output directory
add_custom_command(TARGET Notepad POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<TARGET_FILE_DIR:Notepad>
)
