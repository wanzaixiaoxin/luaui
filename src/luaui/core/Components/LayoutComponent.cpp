#include "Components/LayoutComponent.h"
#include "Control.h"
#include "Components/RenderComponent.h"
#include "Window.h"

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
    // 总是更新渲染矩形（位置可能改变）
    if (m_owner) {
        if (auto* render = m_owner->GetRender()) {
            render->GetRenderRect() = finalRect;
        }
    }
    
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
    if (!m_measureValid) return;  // 已经失效，避免重复冒泡
    
    m_measureValid = false;
    m_arrangeValid = false;
    m_dirty = LayoutDirty::Measure;
    
    // 冒泡到父控件
    if (m_owner) {
        if (auto parent = m_owner->GetParent()) {
            if (auto* parentLayout = static_cast<Control*>(parent.get())->AsLayoutable()) {
                parentLayout->InvalidateMeasure();
            }
        } else {
            // 没有父控件，说明是根控件，通知窗口需要重新布局
            if (auto* window = m_owner->GetWindow()) {
                window->InvalidateLayout();
            }
        }
    }
}

void LayoutComponent::InvalidateArrange() {
    if (!m_arrangeValid) return;  // 已经失效，避免重复冒泡
    
    m_arrangeValid = false;
    m_dirty = LayoutDirty::Arrange;
    
    // 冒泡到父控件
    if (m_owner) {
        if (auto parent = m_owner->GetParent()) {
            if (auto* parentLayout = static_cast<Control*>(parent.get())->AsLayoutable()) {
                parentLayout->InvalidateArrange();
            }
        } else {
            // 没有父控件，说明是根控件，通知窗口需要重新布局
            if (auto* window = m_owner->GetWindow()) {
                window->InvalidateLayout();
            }
        }
    }
}

rendering::Size LayoutComponent::MeasureOverride(const rendering::Size& availableSize) {
    // 默认实现：返回固定大小或约束大小
    if (m_width > 0 && m_height > 0) {
        return rendering::Size(m_width, m_height);
    }
    
    // 尝试调用 Control 的 OnMeasure 方法
    if (m_owner) {
        return m_owner->OnMeasure(availableSize);
    }
    
    return rendering::Size(0, 0);
}

rendering::Size LayoutComponent::ArrangeOverride(const rendering::Size& finalSize) {
    // 默认实现：返回最终大小
    return finalSize;
}

} // namespace components
} // namespace luaui
