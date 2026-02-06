# Input Module - 输入系统

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_Input)

# Placeholder for future input system implementation
set(HEADERS
)

set(SOURCES
)

if(NOT SOURCES)
    add_library(${MODULE_NAME} INTERFACE)
    message(STATUS "Input module: No sources yet, creating interface library")
else()
    add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

    target_include_directories(${MODULE_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)

    target_link_libraries(${MODULE_NAME} PUBLIC
        LuaUI_Core
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
