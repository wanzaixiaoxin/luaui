# Rendering Module - Direct2D based rendering engine

cmake_minimum_required(VERSION 3.15)

set(MODULE_NAME LuaUI_Rendering)

# Source files
set(SOURCES
    d2d/D2DRenderContext.cpp
    d2d/D2DRenderEngine.cpp
    d2d/D2DRenderTarget.cpp
    d2d/D2DBrush.cpp
    d2d/D2DGeometry.cpp
    d2d/D2DBitmap.cpp
    d2d/D2DTextFormat.cpp
    d2d/D2DTextLayoutAdvanced.cpp
    d2d/D2DAnimation.cpp
)

set(HEADERS
    Types.h
    IBrush.h
    IGeometry.h
    IBitmap.h
    ITextFormat.h
    IRenderContext.h
    IRenderEngine.h
    IRenderTarget.h
    ITextLayout.h
    IAnimation.h
    ResourceCache.h
    IFontManager.h
    d2d/D2DRenderContext.h
    d2d/D2DRenderEngine.h
    d2d/D2DRenderTarget.h
    d2d/D2DBrush.h
    d2d/D2DGeometry.h
    d2d/D2DBitmap.h
    d2d/D2DTextFormat.h
    d2d/D2DTextLayoutAdvanced.h
    d2d/D2DAnimation.h
    d2d/D2DHelpers.h
)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/d2d>
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

install(FILES ${HEADERS} DESTINATION include)
