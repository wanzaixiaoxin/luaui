#pragma once
#include "luaui/UIElement.h"
#include <vector>
#include <algorithm>

namespace luaui {

// Panel - Base class for container elements
class Panel : public UIElement {
public:
    Panel();
    virtual ~Panel() = default;
    
    // Children collection
    std::vector<UIElementPtr> Children;
    
    // Add a child
    void AddChild(const UIElementPtr& child);
    
    // Remove a child
    void RemoveChild(const UIElementPtr& child);
    void RemoveChild(size_t index);
    
    // Clear all children
    void ClearChildren();
    
    // Get child count
    size_t GetChildCount() const { return Children.size(); }
    
    // Insert child at position
    void InsertChild(size_t index, const UIElementPtr& child);
    
    // Get child at index
    UIElementPtr GetChild(size_t index) const {
        return (index < Children.size()) ? Children[index] : nullptr;
    }
    
    // Background for the panel
    Color PanelBackground = Color::Transparent();
    
protected:
    // Default Panel implementation measures all children and returns max size
    Size MeasureCore(const Size& available) override;
    void ArrangeCore(const Rect& finalRect) override;
    
    // Helper: Measure all children
    void MeasureChildren(const Size& available);
    
    // Helper: Get combined desired size of all children
    Size GetCombinedChildSize() const;
    
    // Helper: Arrange child with alignment
    void ArrangeChild(const UIElementPtr& child, const Rect& slot);
};

// Shared pointer type
using PanelPtr = std::shared_ptr<Panel>;

} // namespace luaui
