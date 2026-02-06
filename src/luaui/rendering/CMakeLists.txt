# Rendering Module - 渲染引擎

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_Rendering)

# Placeholder for future Direct2D rendering implementation
set(HEADERS
)

set(SOURCES
)

if(NOT SOURCES)
    add_library(${MODULE_NAME} INTERFACE)
    message(STATUS "Rendering module: No sources yet, creating interface library")
else()
    add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

    target_include_directories(${MODULE_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)

    # Windows SDK dependencies
    if(WIN32)
        target_link_libraries(${MODULE_NAME} PUBLIC
            d2d1
            d3d11
            dxgi
            dwrite
            dcomp
            windowscodecs
        )
    endif()

    # Dependencies
    target_link_libraries(${MODULE_NAME} PUBLIC
        LuaUI_Core
        LuaUI_Utils
    )

    install(TARGETS ${MODULE_NAME}
        EXPORT LuaUITargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )

    install(DIRECTORY include/ DESTINATION include)
endif()

if(BUILD_TESTS AND SOURCES)
    add_subdirectory(tests)
endif()
