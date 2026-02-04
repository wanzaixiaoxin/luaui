# Compiler settings for LuaUI project

if(MSVC)
    # Visual Studio specific settings
    add_compile_options(/W4)
    add_compile_options(/MT)
    
    # Disable warnings that are not relevant
    add_compile_options(/wd4996)  # Deprecated functions
    add_compile_options(/wd4100)  # Unreferenced formal parameter
    
    # Enable parallel builds
    add_compile_options(/MP)
    
    # Treat warnings as errors in release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/WX)
    endif()
else()
    # GCC/Clang specific settings
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(-fno-exceptions)
    
    # Treat warnings as errors
    add_compile_options(-Werror)
endif()
