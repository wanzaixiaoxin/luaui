#include "UIElement.h"
#include <algorithm>
#include <cmath>

namespace luaui {

UIElement::UIElement() = default;

void UIElement::Measure(const Size& available) {
    // Store available size
    m_availableSize = available;
    
    // Calculate constraint size based on Width/Height properties
    Size constraint = GetConstraintSize();
    
    // Apply constraint to available size
    Size childAvailable = available;
    if (!IsNaN(constraint.Width)) {
        childAvailable.Width = std::min(available.Width, constraint.Width);
    }
    if (!IsNaN(constraint.Height)) {
        childAvailable.Height = std::min(available.Height, constraint.Height);
    }
    
    // Call core measure
    Size desired = MeasureCore(childAvailable);
    
    // Apply constraints to desired size
    desired = ApplyConstraints(desired);
    
    // Handle explicit Width/Height
    if (!IsNaN(constraint.Width)) {
        desired.Width = constraint.Width;
    }
    if (!IsNaN(constraint.Height)) {
        desired.Height = constraint.Height;
    }
    
    // Clamp to available size
    desired.Width = std::min(desired.Width, available.Width);
    desired.Height = std::min(desired.Height, available.Height);
    
    DesiredSize = desired;
    m_measureValid = true;
    m_arrangeValid = false;  // Arrange becomes invalid after measure
}

void UIElement::Arrange(const Rect& slot) {
    // Store slot
    RenderSlot = slot;
    
    // Calculate final rect (respecting alignment)
    Size finalSize = slot.GetSize();
    
    // Call core arrange
    ArrangeCore(slot);
    
    m_arrangeValid = true;
}

void UIElement::InvalidateMeasure() {
    m_measureValid = false;
    m_arrangeValid = false;
    
    // Invalidate parent too
    if (Parent) {
        Parent->InvalidateMeasure();
    }
}

void UIElement::InvalidateArrange() {
    m_arrangeValid = false;
}

Size UIElement::GetConstraintSize() const {
    return Size(Width, Height);
}

UIElement* UIElement::GetRoot() {
    UIElement* current = this;
    while (current->Parent) {
        current = current->Parent;
    }
    return current;
}

bool UIElement::HitTest(const Point& point) const {
    if (!IsVisible) return false;
    
    float x = point.X;
    float y = point.Y;
    
    return x >= RenderSlot.X && 
           x < RenderSlot.X + RenderSlot.Width &&
           y >= RenderSlot.Y && 
           y < RenderSlot.Y + RenderSlot.Height;
}

Size UIElement::ApplyConstraints(const Size& size) const {
    return Size{
        std::max(MinWidth, std::min(size.Width, MaxWidth)),
        std::max(MinHeight, std::min(size.Height, MaxHeight))
    };
}

} // namespace luaui
