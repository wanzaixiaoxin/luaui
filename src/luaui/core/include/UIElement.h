#pragma once
#include "Types.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace luaui {

// Forward declarations
class Panel;

// UIElement - Base class for all visual elements
class UIElement : public std::enable_shared_from_this<UIElement> {
public:
    UIElement();
    virtual ~UIElement() = default;
    
    // Name for debugging
    std::string Name;
    
    // Layout properties
    float Width = NaN;                    // NaN = Auto
    float Height = NaN;                   // NaN = Auto
    float MinWidth = 0;
    float MinHeight = 0;
    float MaxWidth = Size::Infinity;
    float MaxHeight = Size::Infinity;
    Thickness Margin;
    HorizontalAlignment HAlign = HorizontalAlignment::Stretch;
    VerticalAlignment VAlign = VerticalAlignment::Stretch;
    
    // State
    bool IsVisible = true;
    bool IsEnabled = true;
    
    // Layout results (read-only)
    Size DesiredSize;                     // Result of Measure
    Rect RenderSlot;                      // Result of Arrange
    
    // Visual properties
    float Opacity = 1.0f;
    Color Background = Color::Transparent();
    
    // Parent reference (weak to avoid cycles)
    UIElement* Parent = nullptr;
    
    // === Layout System ===
    
    // Measure: Calculate desired size given available space
    void Measure(const Size& available);
    
    // Arrange: Position element in final slot
    void Arrange(const Rect& slot);
    
    // Invalidate layout (mark dirty)
    void InvalidateMeasure();
    void InvalidateArrange();
    
    // Check if layout is dirty
    bool IsMeasureValid() const { return m_measureValid; }
    bool IsArrangeValid() const { return m_arrangeValid; }
    
    // === Helper methods ===
    
    // Get actual position/size after layout
    Point GetPosition() const { return RenderSlot.Position(); }
    Size GetActualSize() const { return RenderSlot.GetSize(); }
    
    // Get final size (respecting Width/Height constraints)
    Size GetConstraintSize() const;
    
    // Find root
    UIElement* GetRoot();
    
    // Hit test
    virtual bool HitTest(const Point& point) const;
    
    // === Virtual methods for subclasses ===
    
    // Called when element is attached to visual tree
    virtual void OnAttached() {}
    
    // Called when element is detached from visual tree
    virtual void OnDetached() {}
    
    // Render (override in derived classes)
    virtual void Render(void* context) { (void)context; }
    
protected:
    // Core layout logic - subclasses must implement
    virtual Size MeasureCore(const Size& available) = 0;
    virtual void ArrangeCore(const Rect& finalRect) = 0;
    
    // Apply Min/Max constraints to a size
    Size ApplyConstraints(const Size& size) const;
    
private:
    bool m_measureValid = false;
    bool m_arrangeValid = false;
    Size m_availableSize;  // Last available size passed to Measure
};

// Shared pointer type
using UIElementPtr = std::shared_ptr<UIElement>;

} // namespace luaui
