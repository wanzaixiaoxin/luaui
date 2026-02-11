#include "Panel.h"
#include "Interfaces/IControl.h"

namespace luaui {
namespace controls {

// ============================================================================
// Panel
// ============================================================================

Panel::Panel() {
    InitializeComponents();
}

void Panel::InitializeComponents() {
    // 添加布局组件
    GetComponents().AddComponent<components::LayoutComponent>(this);
    
    // 添加渲染组件
    GetComponents().AddComponent<components::RenderComponent>(this);
}

std::shared_ptr<interfaces::IControl> Panel::GetChild(size_t index) const {
    if (index < m_children.size()) {
        return m_children[index];
    }
    return nullptr;
}

void Panel::AddChild(const std::shared_ptr<IControl>& child) {
    if (!child) return;
    
    // 从旧父控件移除
    if (auto oldParent = child->GetParent()) {
        if (auto oldPanel = std::dynamic_pointer_cast<Panel>(oldParent)) {
            oldPanel->RemoveChild(child);
        }
    }
    
    child->SetParent(shared_from_this());
    m_children.push_back(child);
    
    // 标记需要重新布局
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void Panel::RemoveChild(const std::shared_ptr<IControl>& child) {
    if (!child) return;
    
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        (*it)->SetParent(nullptr);
        m_children.erase(it);
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void Panel::RemoveChildAt(size_t index) {
    if (index < m_children.size()) {
        m_children[index]->SetParent(nullptr);
        m_children.erase(m_children.begin() + index);
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void Panel::ClearChildren() {
    for (auto& child : m_children) {
        child->SetParent(nullptr);
    }
    m_children.clear();
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void Panel::InsertChild(size_t index, const std::shared_ptr<IControl>& child) {
    if (!child || index > m_children.size()) return;
    
    child->SetParent(shared_from_this());
    m_children.insert(m_children.begin() + index, child);
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void Panel::OnRenderChildren(rendering::IRenderContext* context) {
    // 渲染所有子控件
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        // 尝试转换为可渲染接口
        if (auto* renderable = static_cast<Control*>(child.get())->AsRenderable()) {
            renderable->Render(context);
        }
    }
}

rendering::Size Panel::OnMeasureChildren(const rendering::Size& availableSize) {
    // 默认实现：返回约束大小
    return availableSize;
}

rendering::Size Panel::OnArrangeChildren(const rendering::Size& finalSize) {
    // 默认实现：排列所有子控件为相同大小
    for (auto& child : m_children) {
        if (auto* layoutable = static_cast<Control*>(child.get())->AsLayoutable()) {
            layoutable->Arrange(rendering::Rect(0, 0, finalSize.width, finalSize.height));
        }
    }
    return finalSize;
}

// ============================================================================
// StackPanel
// ============================================================================

StackPanel::StackPanel() {
    // 初始化由 Panel 完成
}

rendering::Size StackPanel::OnMeasureChildren(const rendering::Size& availableSize) {
    float totalWidth = 0;
    float totalHeight = 0;
    float maxWidth = 0;
    float maxHeight = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        auto* layoutable = static_cast<Control*>(child.get())->AsLayoutable();
        if (!layoutable) continue;
        
        interfaces::LayoutConstraint constraint;
        constraint.available = availableSize;
        
        auto childSize = layoutable->Measure(constraint);
        
        if (m_orientation == Orientation::Horizontal) {
            totalWidth += childSize.width;
            maxHeight = std::max(maxHeight, childSize.height);
        } else {
            totalHeight += childSize.height;
            maxWidth = std::max(maxWidth, childSize.width);
        }
    }
    
    // 添加间距
    if (!m_children.empty()) {
        if (m_orientation == Orientation::Horizontal) {
            totalWidth += m_spacing * (m_children.size() - 1);
        } else {
            totalHeight += m_spacing * (m_children.size() - 1);
        }
    }
    
    if (m_orientation == Orientation::Horizontal) {
        return rendering::Size(totalWidth, maxHeight);
    } else {
        return rendering::Size(maxWidth, totalHeight);
    }
}

rendering::Size StackPanel::OnArrangeChildren(const rendering::Size& finalSize) {
    float currentX = 0;
    float currentY = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        auto* layoutable = static_cast<Control*>(child.get())->AsLayoutable();
        if (!layoutable) continue;
        
        auto desiredSize = layoutable->GetDesiredSize();
        
        if (m_orientation == Orientation::Horizontal) {
            layoutable->Arrange(rendering::Rect(currentX, 0, desiredSize.width, finalSize.height));
            currentX += desiredSize.width + m_spacing;
        } else {
            layoutable->Arrange(rendering::Rect(0, currentY, finalSize.width, desiredSize.height));
            currentY += desiredSize.height + m_spacing;
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
