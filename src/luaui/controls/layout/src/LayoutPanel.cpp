#include "luaui/controls/LayoutPanel.h"
#include <algorithm>

namespace luaui {
namespace controls {

ControlPtr Panel::GetChild(size_t index) const {
    if (index < m_children.size()) {
        return m_children[index];
    }
    return nullptr;
}

void Panel::AddChild(const ControlPtr& child) {
    if (!child) return;
    
    // Remove from current parent if any
    auto currentParent = child->GetParent();
    if (currentParent) {
        auto parentPanel = std::dynamic_pointer_cast<Panel>(currentParent);
        if (parentPanel) {
            parentPanel->RemoveChild(child);
        }
    }
    
    child->SetParent(shared_from_this());
    m_children.push_back(child);
    InvalidateMeasure();
}

void Panel::RemoveChild(const ControlPtr& child) {
    if (!child) return;
    
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        (*it)->SetParent(nullptr);
        m_children.erase(it);
        InvalidateMeasure();
    }
}

void Panel::RemoveChildAt(size_t index) {
    if (index < m_children.size()) {
        m_children[index]->SetParent(nullptr);
        m_children.erase(m_children.begin() + index);
        InvalidateMeasure();
    }
}

void Panel::ClearChildren() {
    for (auto& child : m_children) {
        child->SetParent(nullptr);
    }
    m_children.clear();
    InvalidateMeasure();
}

void Panel::InsertChild(size_t index, const ControlPtr& child) {
    if (!child) return;
    
    auto currentParent = child->GetParent();
    if (currentParent) {
        auto parentPanel = std::dynamic_pointer_cast<Panel>(currentParent);
        if (parentPanel) {
            parentPanel->RemoveChild(child);
        }
    }
    
    child->SetParent(shared_from_this());
    m_children.insert(m_children.begin() + std::min(index, m_children.size()), child);
    InvalidateMeasure();
}

void Panel::Render(IRenderContext* context) {
    // Render background
    RenderOverride(context);
    
    // Render children
    for (auto& child : m_children) {
        if (child->GetIsVisible()) {
            child->Render(context);
        }
    }
}

ControlPtr Panel::HitTestPoint(const Point& point) {
    // Check children first (reverse order for Z-order: last on top)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        auto hit = (*it)->HitTestPoint(point);
        if (hit) return hit;
    }
    
    // Check self
    if (HitTest(point)) {
        return shared_from_this();
    }
    
    return nullptr;
}

} // namespace controls
} // namespace luaui
