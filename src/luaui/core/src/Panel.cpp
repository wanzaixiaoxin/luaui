#include "Panel.h"

namespace luaui {

Panel::Panel() = default;

void Panel::AddChild(const UIElementPtr& child) {
    if (!child) return;
    
    // Detach from current parent if any
    if (child->Parent) {
        auto oldPanel = dynamic_cast<Panel*>(child->Parent);
        if (oldPanel) {
            // Remove from old parent
            auto& oldChildren = oldPanel->Children;
            oldChildren.erase(
                std::remove(oldChildren.begin(), oldChildren.end(), child),
                oldChildren.end()
            );
        }
    }
    
    child->Parent = this;
    Children.push_back(child);
    child->OnAttached();
    
    InvalidateMeasure();
}

void Panel::RemoveChild(const UIElementPtr& child) {
    if (!child) return;
    
    auto it = std::find(Children.begin(), Children.end(), child);
    if (it != Children.end()) {
        child->OnDetached();
        child->Parent = nullptr;
        Children.erase(it);
        InvalidateMeasure();
    }
}

void Panel::RemoveChild(size_t index) {
    if (index >= Children.size()) return;
    
    auto child = Children[index];
    child->OnDetached();
    child->Parent = nullptr;
    Children.erase(Children.begin() + index);
    InvalidateMeasure();
}

void Panel::ClearChildren() {
    for (auto& child : Children) {
        child->OnDetached();
        child->Parent = nullptr;
    }
    Children.clear();
    InvalidateMeasure();
}

void Panel::InsertChild(size_t index, const UIElementPtr& child) {
    if (!child) return;
    
    if (child->Parent) {
        auto oldPanel = dynamic_cast<Panel*>(child->Parent);
        if (oldPanel) {
            auto& oldChildren = oldPanel->Children;
            oldChildren.erase(
                std::remove(oldChildren.begin(), oldChildren.end(), child),
                oldChildren.end()
            );
        }
    }
    
    child->Parent = this;
    Children.insert(Children.begin() + std::min(index, Children.size()), child);
    child->OnAttached();
    
    InvalidateMeasure();
}

Size Panel::MeasureCore(const Size& available) {
    // Default implementation: measure all children and return max size
    Size maxSize;
    
    for (auto& child : Children) {
        if (!child->IsVisible) continue;
        
        child->Measure(available);
        const Size& childSize = child->DesiredSize;
        
        maxSize.Width = std::max(maxSize.Width, childSize.Width);
        maxSize.Height = std::max(maxSize.Height, childSize.Height);
    }
    
    return maxSize;
}

void Panel::ArrangeCore(const Rect& finalRect) {
    // Default implementation: arrange all children to fill the final rect
    for (auto& child : Children) {
        if (!child->IsVisible) continue;
        
        ArrangeChild(child, finalRect);
    }
}

void Panel::MeasureChildren(const Size& available) {
    for (auto& child : Children) {
        if (child->IsVisible) {
            child->Measure(available);
        }
    }
}

Size Panel::GetCombinedChildSize() const {
    Size maxSize;
    for (auto& child : Children) {
        if (!child->IsVisible) continue;
        maxSize.Width = std::max(maxSize.Width, child->DesiredSize.Width);
        maxSize.Height = std::max(maxSize.Height, child->DesiredSize.Height);
    }
    return maxSize;
}

void Panel::ArrangeChild(const UIElementPtr& child, const Rect& slot) {
    if (!child) return;
    
    // Apply alignment
    Rect finalRect = AlignRect(child->DesiredSize, slot, child->HAlign, child->VAlign);
    child->Arrange(finalRect);
}

} // namespace luaui
