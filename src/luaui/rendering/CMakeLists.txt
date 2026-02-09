# Rendering Module - Direct2D based rendering engine

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_Rendering)

# Source files
set(SOURCES
    src/d2d/D2DRenderContext.cpp
    src/d2d/D2DRenderEngine.cpp
    src/d2d/D2DRenderTarget.cpp
    src/d2d/D2DBrush.cpp
    src/d2d/D2DGeometry.cpp
    src/d2d/D2DBitmap.cpp
    src/d2d/D2DTextFormat.cpp
    src/d2d/D2DTextLayoutAdvanced.cpp
    src/d2d/D2DAnimation.cpp
)

set(HEADERS
    include/Types.h
    include/IBrush.h
    include/IGeometry.h
    include/IBitmap.h
    include/ITextFormat.h
    include/IRenderContext.h
    include/IRenderEngine.h
    include/IRenderTarget.h
    include/ITextLayout.h
    include/IAnimation.h
    include/ResourceCache.h
    include/IFontManager.h
    src/d2d/D2DRenderContext.h
    src/d2d/D2DRenderEngine.h
    src/d2d/D2DRenderTarget.h
    src/d2d/D2DBrush.h
    src/d2d/D2DGeometry.h
    src/d2d/D2DBitmap.h
    src/d2d/D2DTextFormat.h
    src/d2d/D2DTextLayoutAdvanced.h
    src/d2d/D2DAnimation.h
    src/d2d/D2DHelpers.h
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
    LuaUI_Utils
)

# Windows SDK dependencies
if(WIN32)
    target_link_libraries(${MODULE_NAME} PUBLIC
        d2d1
        dwrite
        windowscodecs
        winmm
        ws2_32
        dxgi
    )
endif()

install(TARGETS ${MODULE_NAME}
    EXPORT LuaUITargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
