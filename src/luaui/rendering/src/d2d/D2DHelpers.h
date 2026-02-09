#pragma once

#include "Types.h"
#include <d2d1.h>
#include <wincodec.h>

namespace luaui {
namespace rendering {

// Convert Color to D2D_COLOR_F
inline D2D1_COLOR_F ToD2DColor(const Color& color) {
    return D2D1::ColorF(color.r, color.g, color.b, color.a);
}

// Convert Point to D2D1_POINT_2F
inline D2D1_POINT_2F ToD2DPoint(const Point& point) {
    return D2D1::Point2F(point.x, point.y);
}

// Convert Rect to D2D1_RECT_F
inline D2D1_RECT_F ToD2DRect(const Rect& rect) {
    return D2D1::RectF(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
}

// Convert Size to D2D1_SIZE_F
inline D2D1_SIZE_F ToD2DSize(const Size& size) {
    return D2D1::SizeF(size.width, size.height);
}

} // namespace rendering
} // namespace luaui
