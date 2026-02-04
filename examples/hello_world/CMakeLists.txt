# Hello World example

file(GLOB HELLO_WORLD_SOURCES "*.cpp")

add_executable(HelloWorld ${HELLO_WORLD_SOURCES})

target_include_directories(HelloWorld PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(HelloWorld
    LuaUI_Core
    LuaUI_XML
    LuaUI_UI
    LuaUI_Lua
    LuaUI_Resource
    LuaUI_Utils
)

# Copy Lua and XML files to output directory
add_custom_command(TARGET HelloWorld POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<TARGET_FILE_DIR:HelloWorld>
)
