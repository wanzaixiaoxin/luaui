#include "Components/LayoutComponent.h"
#include "Control.h"

namespace luaui {
namespace components {

LayoutComponent::LayoutComponent(Control* owner) : Component(owner) {}

rendering::Size LayoutComponent::Measure(const LayoutConstraint& constraint) {
    if (!IsMeasureValid() || 
        constraint.available.width != m_lastAvailableSize.width ||
        constraint.available.height != m_lastAvailableSize.height) {
        m_desiredSize = MeasureOverride(constraint.available);
        m_lastAvailableSize = constraint.available;
        m_lastConstraint = constraint;
        m_measureValid = true;
    }
    return m_desiredSize;
}

void LayoutComponent::Arrange(const rendering::Rect& finalRect) {
    if (!IsArrangeValid()) {
        ArrangeOverride(rendering::Size(finalRect.width, finalRect.height));
        m_arrangeValid = true;
    }
}

void LayoutComponent::SetWidth(float width) {
    m_width = width;
    InvalidateMeasure();
}

void LayoutComponent::SetHeight(float height) {
    m_height = height;
    InvalidateMeasure();
}

void LayoutComponent::SetMargin(float left, float top, float right, float bottom) {
    m_marginLeft = left;
    m_marginTop = top;
    m_marginRight = right;
    m_marginBottom = bottom;
    InvalidateMeasure();
}

void LayoutComponent::SetPadding(float left, float top, float right, float bottom) {
    m_paddingLeft = left;
    m_paddingTop = top;
    m_paddingRight = right;
    m_paddingBottom = bottom;
    InvalidateMeasure();
}

void LayoutComponent::SetHorizontalAlignment(HorizontalAlignment align) {
    m_hAlignment = align;
    InvalidateArrange();
}

void LayoutComponent::SetVerticalAlignment(VerticalAlignment align) {
    m_vAlignment = align;
    InvalidateArrange();
}

void LayoutComponent::InvalidateMeasure() {
    m_measureValid = false;
    m_arrangeValid = false;
    m_dirty = LayoutDirty::Measure;
    // TODO: 冒泡到父控件
}

void LayoutComponent::InvalidateArrange() {
    m_arrangeValid = false;
    m_dirty = LayoutDirty::Arrange;
    // TODO: 冒泡到父控件
}

rendering::Size LayoutComponent::MeasureOverride(const rendering::Size& availableSize) {
    // 默认实现：返回固定大小或约束大小
    if (m_width > 0 && m_height > 0) {
        return rendering::Size(m_width, m_height);
    }
    return rendering::Size(0, 0);
}

rendering::Size LayoutComponent::ArrangeOverride(const rendering::Size& finalSize) {
    // 默认实现：返回最终大小
    return finalSize;
}

} // namespace components
} // namespace luaui
