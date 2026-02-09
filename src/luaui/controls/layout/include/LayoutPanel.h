#pragma once
#include "Control.h"
#include <vector>

namespace luaui {
namespace controls {

// Panel - Container base class for layout
class Panel : public Control {
public:
    Panel() = default;
    virtual ~Panel() = default;
    
    std::string GetTypeName() const override { return "Panel"; }
    
    // Children access
    size_t GetChildCount() const override { return m_children.size(); }
    ControlPtr GetChild(size_t index) const override;
    const std::vector<ControlPtr>& GetChildren() const { return m_children; }
    
    // Child management
    virtual void AddChild(const ControlPtr& child);
    virtual void RemoveChild(const ControlPtr& child);
    virtual void RemoveChildAt(size_t index);
    virtual void ClearChildren();
    
    // Insert at specific position
    virtual void InsertChild(size_t index, const ControlPtr& child);
    
    // Rendering
    void Render(IRenderContext* context) override;
    
    // Hit testing - search children first (Z-order: last child on top)
    ControlPtr HitTestPoint(const Point& point) override;
    
protected:
    std::vector<ControlPtr> m_children;
};

} // namespace controls
} // namespace luaui
