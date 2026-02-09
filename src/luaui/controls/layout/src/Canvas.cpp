#include "Canvas.h"
#include <algorithm>
#include <unordered_map>
#include <limits>

namespace luaui {
namespace controls {

static std::unordered_map<Control*, float> s_leftMap;
static std::unordered_map<Control*, float> s_topMap;
static std::unordered_map<Control*, float> s_rightMap;
static std::unordered_map<Control*, float> s_bottomMap;

static float NaN() { return std::numeric_limits<float>::quiet_NaN(); }
static bool IsNaN(float v) { return std::isnan(v); }

void Canvas::SetLeft(Control* control, float left) {
    if (control) s_leftMap[control] = left;
}

void Canvas::SetTop(Control* control, float top) {
    if (control) s_topMap[control] = top;
}

void Canvas::SetRight(Control* control, float right) {
    if (control) s_rightMap[control] = right;
}

void Canvas::SetBottom(Control* control, float bottom) {
    if (control) s_bottomMap[control] = bottom;
}

float Canvas::GetLeft(Control* control) {
    if (!control) return 0;
    auto it = s_leftMap.find(control);
    return (it != s_leftMap.end()) ? it->second : 0;
}

float Canvas::GetTop(Control* control) {
    if (!control) return 0;
    auto it = s_topMap.find(control);
    return (it != s_topMap.end()) ? it->second : 0;
}

float Canvas::GetRight(Control* control) {
    if (!control) return NaN();
    auto it = s_rightMap.find(control);
    return (it != s_rightMap.end()) ? it->second : NaN();
}

float Canvas::GetBottom(Control* control) {
    if (!control) return NaN();
    auto it = s_bottomMap.find(control);
    return (it != s_bottomMap.end()) ? it->second : NaN();
}

Size Canvas::MeasureOverride(const Size& availableSize) {
    Size desired;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        child->Measure(Size(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()));
        
        float left = GetLeft(child.get());
        float top = GetTop(child.get());
        float right = GetRight(child.get());
        float bottom = GetBottom(child.get());
        
        Size childSize = child->GetDesiredSize();
        
        float childRight = IsNaN(right) ? left + childSize.width : availableSize.width - right;
        float childBottom = IsNaN(bottom) ? top + childSize.height : availableSize.height - bottom;
        
        desired.width = std::max(desired.width, childRight);
        desired.height = std::max(desired.height, childBottom);
    }
    
    return desired;
}

Size Canvas::ArrangeOverride(const Size& finalSize) {
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        float left = GetLeft(child.get());
        float top = GetTop(child.get());
        float right = GetRight(child.get());
        float bottom = GetBottom(child.get());
        
        Size childSize = child->GetDesiredSize();
        
        if (!IsNaN(right)) {
            left = finalSize.width - right - childSize.width;
        }
        if (!IsNaN(bottom)) {
            top = finalSize.height - bottom - childSize.height;
        }
        
        child->Arrange(Rect(left, top, childSize.width, childSize.height));
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
