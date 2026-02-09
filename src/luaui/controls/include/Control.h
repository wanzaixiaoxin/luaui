#pragma once

#include "luaui/rendering/Types.h"
#include "luaui/rendering/IRenderContext.h"
#include "luaui/controls/Event.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <any>

namespace luaui {
namespace controls {

using namespace luaui::rendering;

// Forward declarations
class Control;
class Panel;

using ControlPtr = std::shared_ptr<Control>;
using ConstControlPtr = std::shared_ptr<const Control>;

// Property change callback
using PropertyChangedCallback = std::function<void(Control* control, const std::string& propertyName)>;

// Property metadata
struct PropertyMetadata {
    std::any defaultValue;
    PropertyChangedCallback changedCallback;
    bool affectsMeasure = false;
    bool affectsArrange = false;
    bool affectsRender = true;
};

// Dependency Property
class DependencyProperty {
public:
    using Id = uint32_t;
    
    static Id Register(const std::string& name, const PropertyMetadata& metadata);
    static DependencyProperty* GetById(Id id);
    static DependencyProperty* GetByName(const std::string& name);
    
    const std::string& GetName() const { return m_name; }
    Id GetId() const { return m_id; }
    const PropertyMetadata& GetMetadata() const { return m_metadata; }
    
    // Constructor is public for std::make_unique
    DependencyProperty(const std::string& name, const PropertyMetadata& metadata);
    
    std::string m_name;
    Id m_id;
    PropertyMetadata m_metadata;
    
    static Id s_nextId;
    static std::map<Id, std::unique_ptr<DependencyProperty>> s_properties;
    static std::map<std::string, DependencyProperty*> s_nameMap;
};

// Control base class
class Control : public std::enable_shared_from_this<Control> {
public:
    Control();
    virtual ~Control();
    
    // Hierarchy
    virtual ControlPtr GetParent() const { return m_parent.lock(); }
    virtual void SetParent(const ControlPtr& parent) { m_parent = parent; }
    
    virtual Panel* GetParentPanel() const;
    
    virtual size_t GetChildCount() const { return 0; }
    virtual ControlPtr GetChild(size_t /*index*/) const { return nullptr; }
    
    // Identity
    virtual const std::string& GetName() const { return m_name; }
    virtual void SetName(const std::string& name) { m_name = name; }
    
    virtual std::string GetTypeName() const = 0;
    
    // Dependency Properties
    virtual std::any GetValue(DependencyProperty::Id propertyId) const;
    virtual void SetValue(DependencyProperty::Id propertyId, const std::any& value);
    virtual void ClearValue(DependencyProperty::Id propertyId);
    virtual bool HasLocalValue(DependencyProperty::Id propertyId) const;
    
    // Layout
    virtual void Measure(const Size& availableSize);
    virtual void Arrange(const Rect& finalRect);
    
    Size GetDesiredSize() const { return m_desiredSize; }
    Rect GetRenderRect() const { return m_renderRect; }
    
    void InvalidateMeasure();
    void InvalidateArrange();
    
    // Hit testing
    virtual bool HitTest(const Point& point) const;
    virtual ControlPtr HitTestPoint(const Point& point);
    
    // Rendering
    virtual void Render(IRenderContext* context);
    
    // Visibility
    virtual bool GetIsVisible() const { return m_isVisible; }
    virtual void SetIsVisible(bool visible);
    
    virtual float GetOpacity() const { return m_opacity; }
    virtual void SetOpacity(float opacity);
    
    // Margin and Padding
    virtual float GetMarginLeft() const { return m_marginLeft; }
    virtual float GetMarginTop() const { return m_marginTop; }
    virtual float GetMarginRight() const { return m_marginRight; }
    virtual float GetMarginBottom() const { return m_marginBottom; }
    virtual void SetMargin(float left, float top, float right, float bottom);
    
    virtual float GetPaddingLeft() const { return m_paddingLeft; }
    virtual float GetPaddingTop() const { return m_paddingTop; }
    virtual float GetPaddingRight() const { return m_paddingRight; }
    virtual float GetPaddingBottom() const { return m_paddingBottom; }
    virtual void SetPadding(float left, float top, float right, float bottom);
    
    // Size constraints
    virtual float GetWidth() const { return m_width; }
    virtual float GetHeight() const { return m_height; }
    virtual void SetWidth(float width);
    virtual void SetHeight(float height);
    
    virtual float GetMinWidth() const { return m_minWidth; }
    virtual float GetMinHeight() const { return m_minHeight; }
    virtual void SetMinWidth(float value) { m_minWidth = value; }
    virtual void SetMinHeight(float value) { m_minHeight = value; }
    
    virtual float GetMaxWidth() const { return m_maxWidth; }
    virtual float GetMaxHeight() const { return m_maxHeight; }
    virtual void SetMaxWidth(float value) { m_maxWidth = value; }
    virtual void SetMaxHeight(float value) { m_maxHeight = value; }
    
    // Alignment
    enum class HorizontalAlignment { Left, Center, Right, Stretch };
    enum class VerticalAlignment { Top, Center, Bottom, Stretch };
    
    HorizontalAlignment GetHorizontalAlignment() const { return m_hAlignment; }
    VerticalAlignment GetVerticalAlignment() const { return m_vAlignment; }
    void SetHorizontalAlignment(HorizontalAlignment align);
    void SetVerticalAlignment(VerticalAlignment align);
    
    // Background
    virtual Color GetBackground() const { return m_background; }
    virtual void SetBackground(const Color& color);
    
    // Render transform
    virtual const Transform& GetRenderTransform() const { return m_renderTransform; }
    virtual void SetRenderTransform(const Transform& transform) { m_renderTransform = transform; }
    
    // Events
    using EventHandler = std::function<void(Control* sender)>;
    
    void AddClickHandler(EventHandler handler) { m_clickHandlers.push_back(handler); }
    void ClearClickHandlers() { m_clickHandlers.clear(); }
    void RaiseClick();
    
    void AddMouseEnterHandler(EventHandler handler) { m_mouseEnterHandlers.push_back(handler); }
    void AddMouseLeaveHandler(EventHandler handler) { m_mouseLeaveHandlers.push_back(handler); }
    void AddMouseMoveHandler(std::function<void(Control*, Point)> handler) { m_mouseMoveHandlers.push_back(handler); }
    
    void RaiseMouseEnter() { for (auto& h : m_mouseEnterHandlers) h(this); }
    void RaiseMouseLeave() { for (auto& h : m_mouseLeaveHandlers) h(this); }
    void RaiseMouseMove(const Point& p) { for (auto& h : m_mouseMoveHandlers) h(this, p); }
    
    // ==================== 路由事件系统 ====================
    // Event handler function type
    using RoutedEventHandler = std::function<void(Control* sender, RoutedEventArgs& args)>;
    
    // Add event handler for routed events
    void AddHandler(const RoutedEvent& routedEvent, RoutedEventHandler handler);
    
    // Remove event handler
    void RemoveHandler(const RoutedEvent& routedEvent);
    
    // Raise routed event
    void RaiseEvent(const RoutedEvent& routedEvent, RoutedEventArgs& args);
    
    // Virtual event handlers (override in derived classes)
    virtual void OnPreviewMouseDown(MouseEventArgs& /*args*/) {}
    virtual void OnMouseDown(MouseEventArgs& /*args*/) {}
    virtual void OnPreviewMouseUp(MouseEventArgs& /*args*/) {}
    virtual void OnMouseUp(MouseEventArgs& /*args*/) {}
    virtual void OnPreviewMouseMove(MouseEventArgs& /*args*/) {}
    virtual void OnMouseMove(MouseEventArgs& /*args*/) {}
    virtual void OnMouseEnter() {}
    virtual void OnMouseLeave() {}
    virtual void OnMouseWheel(MouseEventArgs& /*args*/) {}
    
    virtual void OnPreviewKeyDown(KeyEventArgs& /*args*/) {}
    virtual void OnKeyDown(KeyEventArgs& /*args*/) {}
    virtual void OnPreviewKeyUp(KeyEventArgs& /*args*/) {}
    virtual void OnKeyUp(KeyEventArgs& /*args*/) {}
    
    virtual void OnPreviewGotFocus(FocusEventArgs& /*args*/) {}
    virtual void OnGotFocus() {}
    virtual void OnPreviewLostFocus(FocusEventArgs& /*args*/) {}
    virtual void OnLostFocus() {}
    
    // Focus
    virtual bool GetIsFocusable() const { return m_isFocusable; }
    virtual void SetIsFocusable(bool focusable);
    virtual bool GetIsFocused() const { return m_isFocused; }
    virtual bool Focus();
    virtual void KillFocus();
    
    // Dirty state
    virtual void Invalidate();
    virtual bool IsDirty() const { return m_isDirty; }
    virtual void ClearDirty() { m_isDirty = false; }
    
    // Data context
    virtual std::any GetDataContext() const { return m_dataContext; }
    virtual void SetDataContext(const std::any& data) { m_dataContext = data; }
    
protected:
    // Attached properties
    static void SetAttachedProperty(Control* control, const std::string& key, const std::any& value);
    static std::any GetAttachedProperty(Control* control, const std::string& key);
    
    // Layout overrides
    virtual Size MeasureOverride(const Size& availableSize);
    virtual Size ArrangeOverride(const Size& finalSize);
    
    // Render override
    virtual void RenderOverride(IRenderContext* context);
    
    // Property change
    virtual void OnPropertyChanged(const std::string& propertyName);
    
    void SetActualSize(float width, float height);
    
    // Hierarchy
    std::weak_ptr<Control> m_parent;
    
    // Identity
    std::string m_name;
    
    // Layout state
    Size m_desiredSize;
    Rect m_renderRect;
    Size m_lastAvailableSize;
    bool m_needsMeasure = true;
    bool m_needsArrange = true;
    
    // Size
    float m_width = 0;      // 0 = Auto
    float m_height = 0;     // 0 = Auto
    float m_minWidth = 0;
    float m_minHeight = 0;
    float m_maxWidth = std::numeric_limits<float>::max();
    float m_maxHeight = std::numeric_limits<float>::max();
    float m_actualWidth = 0;
    float m_actualHeight = 0;
    
    // Margin/Padding
    float m_marginLeft = 0, m_marginTop = 0, m_marginRight = 0, m_marginBottom = 0;
    float m_paddingLeft = 0, m_paddingTop = 0, m_paddingRight = 0, m_paddingBottom = 0;
    
    // Alignment
    HorizontalAlignment m_hAlignment = HorizontalAlignment::Stretch;
    VerticalAlignment m_vAlignment = VerticalAlignment::Stretch;
    
    // Transform
    Transform m_renderTransform;
    
    // Appearance
    bool m_isVisible = true;
    float m_opacity = 1.0f;
    Color m_background = Color::Transparent();
    
    // Focus
    bool m_isFocusable = false;
    bool m_isFocused = false;
    
    // Dirty state
    bool m_isDirty = true;
    
    // Data
    std::any m_dataContext;
    
private:
    // Allow FocusManager to access private members
    friend class FocusManager;
    
    // Effective values storage
    std::map<DependencyProperty::Id, std::any> m_effectiveValues;
    
    // Event handlers
    std::vector<EventHandler> m_clickHandlers;
    std::vector<EventHandler> m_mouseEnterHandlers;
    std::vector<EventHandler> m_mouseLeaveHandlers;
    std::vector<std::function<void(Control*, Point)>> m_mouseMoveHandlers;
    
    // Routed event handlers storage
    std::map<size_t, std::vector<RoutedEventHandler>> m_routedEventHandlers;
    
    // Attached properties storage
    std::map<std::string, std::any> m_attachedProperties;
};

// Panel (container with multiple children)
class Panel : public Control {
public:
    Panel() = default;
    virtual ~Panel() = default;
    
    size_t GetChildCount() const override { return m_children.size(); }
    ControlPtr GetChild(size_t index) const override;
    
    virtual void AddChild(const ControlPtr& child);
    virtual void RemoveChild(const ControlPtr& child);
    virtual void RemoveChildAt(size_t index);
    virtual void ClearChildren();
    
    void Render(IRenderContext* context) override;
    
    // Hit testing - search children first (Z-order: last child on top)
    ControlPtr HitTestPoint(const Point& point) override;
    
protected:
    std::vector<ControlPtr> m_children;
};

// Content control (single child)
class ContentControl : public Control {
public:
    ContentControl() = default;
    virtual ~ContentControl() = default;
    
    ControlPtr GetContent() const { return m_content; }
    virtual void SetContent(const ControlPtr& content);
    
    size_t GetChildCount() const override { return m_content ? 1 : 0; }
    ControlPtr GetChild(size_t index) const override;
    
    void Render(IRenderContext* context) override;
    
protected:
    ControlPtr m_content;
};

// Border (decorator with border)
class Border : public Control {
public:
    Border() = default;
    
    std::string GetTypeName() const override { return "Border"; }
    
    ControlPtr GetContent() const { return m_content; }
    virtual void SetContent(const ControlPtr& content);
    
    size_t GetChildCount() const override { return m_content ? 1 : 0; }
    ControlPtr GetChild(size_t index) const override;
    
    void Render(IRenderContext* context) override;
    
    // Border properties
    float GetBorderThickness() const { return m_borderThickness; }
    void SetBorderThickness(float thickness) { m_borderThickness = thickness; Invalidate(); }
    
    Color GetBorderBrush() const { return m_borderBrush; }
    void SetBorderBrush(const Color& color) { m_borderBrush = color; Invalidate(); }
    
    CornerRadius GetCornerRadius() const { return m_cornerRadius; }
    void SetCornerRadius(const CornerRadius& radius) { m_cornerRadius = radius; Invalidate(); }
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    Size ArrangeOverride(const Size& finalSize) override;
    
protected:
    ControlPtr m_content;
    float m_borderThickness = 1.0f;  // Default 1px border
    Color m_borderBrush = Color::FromHex(0x808080);  // Default gray border
    CornerRadius m_cornerRadius;
};



// Canvas (absolute positioning)
class Canvas : public Panel {
public:
    Canvas() = default;
    
    std::string GetTypeName() const override { return "Canvas"; }
    
    // Attached properties
    static void SetLeft(Control* control, float left);
    static void SetTop(Control* control, float top);
    static float GetLeft(Control* control);
    static float GetTop(Control* control);
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    Size ArrangeOverride(const Size& finalSize) override;
};

// StackPanel
class StackPanel : public Panel {
public:
    StackPanel() = default;
    
    std::string GetTypeName() const override { return "StackPanel"; }
    
    enum class Orientation { Horizontal, Vertical };
    
    Orientation GetOrientation() const { return m_orientation; }
    void SetOrientation(Orientation orient) { m_orientation = orient; InvalidateMeasure(); }
    
    float GetSpacing() const { return m_spacing; }
    void SetSpacing(float spacing) { m_spacing = spacing; InvalidateMeasure(); }
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    Size ArrangeOverride(const Size& finalSize) override;
    
private:
    Orientation m_orientation = Orientation::Vertical;
    float m_spacing = 0;
};

// Grid
class Grid : public Panel {
public:
    Grid() = default;
    
    std::string GetTypeName() const override { return "Grid"; }
    
    // Row/Column definitions
    struct RowDefinition {
        float height = 0;       // 0 = Auto, >1 = Pixel, <1 = Star
        float minHeight = 0;
        float maxHeight = std::numeric_limits<float>::max();
        float actualHeight = 0;
    };
    
    struct ColumnDefinition {
        float width = 0;        // 0 = Auto, >1 = Pixel, <1 = Star
        float minWidth = 0;
        float maxWidth = std::numeric_limits<float>::max();
        float actualWidth = 0;
    };
    
    void AddRowDefinition(float height);
    void AddColumnDefinition(float width);
    void ClearRowDefinitions() { m_rows.clear(); }
    void ClearColumnDefinitions() { m_columns.clear(); }
    
    // Attached properties
    static void SetRow(Control* control, int row);
    static void SetColumn(Control* control, int column);
    static void SetRowSpan(Control* control, int span);
    static void SetColumnSpan(Control* control, int span);
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    Size ArrangeOverride(const Size& finalSize) override;
    
private:
    std::vector<RowDefinition> m_rows;
    std::vector<ColumnDefinition> m_columns;
};

// Button
class Button : public Border {
public:
    Button();
    
    std::string GetTypeName() const override { return "Button"; }
    
    bool GetIsPressed() const { return m_isPressed; }
    
    void Render(IRenderContext* context) override;
    
    // Event handling
    void OnMouseEnter();
    void OnMouseLeave();
    void OnMouseDown(const Point& point);
    void OnMouseUp(const Point& point);
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    void RenderOverride(IRenderContext* context) override;
    
private:
    bool m_isPressed = false;
    bool m_isHovered = false;
    
    Color m_normalBackground = Color::FromHex(0xE0E0E0);
    Color m_hoverBackground = Color::FromHex(0xD0D0D0);
    Color m_pressedBackground = Color::FromHex(0xC0C0C0);
};

// TextBlock
class TextBlock : public Control {
public:
    TextBlock();
    
    std::string GetTypeName() const override { return "TextBlock"; }
    
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    Color GetForeground() const { return m_foreground; }
    void SetForeground(const Color& color) { m_foreground = color; Invalidate(); }
    
    float GetFontSize() const { return m_fontSize; }
    void SetFontSize(float size) { m_fontSize = size; InvalidateMeasure(); }
    
    std::wstring GetFontFamily() const { return m_fontFamily; }
    void SetFontFamily(const std::wstring& family) { m_fontFamily = family; InvalidateMeasure(); }
    
    void Render(IRenderContext* context) override;
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    
private:
    std::wstring m_text;
    Color m_foreground = Color::Black();
    float m_fontSize = 14.0f;
    std::wstring m_fontFamily = L"Segoe UI";
    
    Size m_textSize;  // Cached text size
    bool m_textSizeDirty = true;
    
    void UpdateTextSize(IRenderContext* context);
};

} // namespace controls
} // namespace luaui
