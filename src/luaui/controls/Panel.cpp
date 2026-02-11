#include "Panel.h"
#include "Interfaces/IControl.h"
#include "IRenderContext.h"
#include <iostream>

namespace luaui {
namespace controls {

// ============================================================================
// Panel
// ============================================================================

Panel::Panel() {}

void Panel::InitializeComponents() {
    
    // 调用基类初始化
    Control::InitializeComponents();
    
    // 添加 Panel 专用布局组件（会测量和排列子控件）
    // 注意：GetComponent 使用 typeid 查找，所以 AddComponent 和 GetComponent 必须使用相同类型
    auto* layoutComp = GetComponents().AddComponent<PanelLayoutComponent>(this);
    std::cout << "[Panel] PanelLayoutComponent added: " << (layoutComp ? "yes" : "no") << std::endl;
    
    // 添加 Panel 专用渲染组件（会渲染子控件）
    GetComponents().AddComponent<PanelRenderComponent>(this);
}

// ============================================================================
// PanelLayoutComponent
// ============================================================================

rendering::Size PanelLayoutComponent::MeasureOverride(const rendering::Size& availableSize) {
    // 先让 Panel 测量其子控件
    float childWidth = 0, childHeight = 0;
    if (m_owner) {
        if (auto* panel = dynamic_cast<Panel*>(m_owner)) {
            auto childSize = panel->OnMeasureChildren(availableSize);
            childWidth = childSize.width;
            childHeight = childSize.height;
        }
    }
    
    // 应用固定大小（如果有）
    float w = GetWidth();
    float h = GetHeight();
    
    if (w > 0) {
        // 有固定宽度，使用固定宽度，高度使用子控件高度或固定高度
        if (h > 0) {
            return rendering::Size(w, h);
        } else {
            return rendering::Size(w, childHeight > 0 ? childHeight : availableSize.height);
        }
    } else if (h > 0) {
        // 只有固定高度，使用子控件宽度或可用宽度
        return rendering::Size(childWidth > 0 ? childWidth : availableSize.width, h);
    }
    
    // 没有固定大小，使用子控件测量结果
    if (childWidth > 0 || childHeight > 0) {
        return rendering::Size(childWidth, childHeight);
    }
    
    return availableSize; // 默认使用可用大小
}

rendering::Size PanelLayoutComponent::ArrangeOverride(const rendering::Size& finalSize) {
    // 让 Panel 排列其子控件
    if (auto* panel = dynamic_cast<Panel*>(m_owner)) {
        return panel->OnArrangeChildren(finalSize);
    }
    
    return finalSize;
}

// ============================================================================
// PanelRenderComponent
// ============================================================================

void PanelRenderComponent::RenderOverride(rendering::IRenderContext* context) {
    // 委托给带 localRect 参数的版本
    rendering::Rect localRect(0, 0, m_renderRect.width, m_renderRect.height);
    RenderOverride(context, localRect);
}

void PanelRenderComponent::RenderOverride(rendering::IRenderContext* context, const rendering::Rect& localRect) {
    if (!m_owner || !context) return;
    
    // 1. 调用基类渲染（背景和 OnRender）- 使用本地坐标
    RenderComponent::RenderOverride(context, localRect);
    
    // 2. 渲染子控件（如果 owner 是 Panel）
    if (auto* panel = dynamic_cast<Panel*>(m_owner)) {
        panel->OnRenderChildren(context);
    }
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
        if (!child->GetIsVisible()) {
            std::cout << "  [OnRenderChildren] Child invisible, skipping" << std::endl;
            continue;
        }
        
        // 尝试转换为可渲染接口
        auto* renderable = static_cast<Control*>(child.get())->AsRenderable();
        if (renderable) {
            std::cout << "  [OnRenderChildren] Rendering child..." << std::endl;
            renderable->Render(context);
        } else {
            std::cout << "  [OnRenderChildren] Child not renderable" << std::endl;
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

} // namespace controls
} // namespace luaui
