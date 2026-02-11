#include "Panel.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Interfaces/IRenderable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"

namespace luaui {
namespace controls {

Panel::Panel() {
    InitializeComponents();
}

void Panel::InitializeComponents() {
    Control::InitializeComponents();
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
}

std::shared_ptr<interfaces::IControl> Panel::GetChild(size_t index) const {
    if (index < m_children.size()) {
        return m_children[index];
    }
    return nullptr;
}

void Panel::AddChild(const std::shared_ptr<interfaces::IControl>& child) {
    if (!child) return;
    
    // 从原父控件移除
    if (auto parent = child->GetParent()) {
        if (auto parentPanel = std::dynamic_pointer_cast<Panel>(parent)) {
            parentPanel->RemoveChild(child);
        }
    }
    
    child->SetParent(shared_from_this());
    m_children.push_back(child);
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void Panel::RemoveChild(const std::shared_ptr<interfaces::IControl>& child) {
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

void Panel::InsertChild(size_t index, const std::shared_ptr<interfaces::IControl>& child) {
    if (!child) return;
    
    if (index > m_children.size()) {
        index = m_children.size();
    }
    
    child->SetParent(shared_from_this());
    m_children.insert(m_children.begin() + index, child);
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void Panel::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* renderable = child->AsRenderable()) {
            renderable->Render(context);
        }
    }
}

rendering::Size Panel::OnMeasureChildren(const rendering::Size& availableSize) {
    // 默认实现：测量所有子控件并返回最大尺寸
    float maxWidth = 0;
    float maxHeight = 0;
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            layoutable->Measure(constraint);
            auto desired = layoutable->GetDesiredSize();
            maxWidth = std::max(maxWidth, desired.width);
            maxHeight = std::max(maxHeight, desired.height);
        }
    }
    
    return rendering::Size(maxWidth, maxHeight);
}

rendering::Size Panel::OnArrangeChildren(const rendering::Size& finalSize) {
    // 默认实现：将所有子控件排列在左上角
    auto* render = GetRender();
    if (!render) return finalSize;
    
    auto contentRect = render->GetRenderRect();
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            layoutable->Arrange(rendering::Rect(contentRect.x, contentRect.y,
                                                  desired.width, desired.height));
        }
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
