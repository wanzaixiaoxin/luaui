#include "Control.h"
#include "FocusManager.h"
#include "layout.h"
#include <algorithm>
#include <limits>
#include <cmath>

#ifndef VK_LEFT
#define VK_LEFT     0x25
#define VK_UP       0x26
#define VK_RIGHT    0x27
#define VK_DOWN     0x28
#define VK_HOME     0x24
#define VK_END      0x23
#define VK_DELETE   0x2E
#define VK_TAB      0x09
#define VK_RETURN   0x0D
#define VK_BACK     0x08
#endif

namespace luaui {
namespace controls {

// ==================== DependencyProperty ====================

DependencyProperty::Id DependencyProperty::s_nextId = 1;
std::map<DependencyProperty::Id, std::unique_ptr<DependencyProperty>> DependencyProperty::s_properties;
std::map<std::string, DependencyProperty*> DependencyProperty::s_nameMap;

DependencyProperty::DependencyProperty(const std::string& name, const PropertyMetadata& metadata)
    : m_name(name), m_id(s_nextId++), m_metadata(metadata) {
}

DependencyProperty::Id DependencyProperty::Register(const std::string& name, const PropertyMetadata& metadata) {
    auto prop = std::make_unique<DependencyProperty>(name, metadata);
    Id id = prop->GetId();
    s_nameMap[name] = prop.get();
    s_properties[id] = std::move(prop);
    return id;
}

DependencyProperty* DependencyProperty::GetById(Id id) {
    auto it = s_properties.find(id);
    return it != s_properties.end() ? it->second.get() : nullptr;
}

DependencyProperty* DependencyProperty::GetByName(const std::string& name) {
    auto it = s_nameMap.find(name);
    return it != s_nameMap.end() ? it->second : nullptr;
}

// ==================== Control ====================

Control::Control() {
    SetIsFocusable(true);
}

Control::~Control() {
    // 从FocusManager注销，避免悬空指�?
    FocusManager::GetInstance().UnregisterFocusable(this);
}

Panel* Control::GetParentPanel() const {
    auto parent = GetParent();
    return parent ? dynamic_cast<Panel*>(parent.get()) : nullptr;
}

// ==================== Property System ====================

std::any Control::GetValue(DependencyProperty::Id propertyId) const {
    auto it = m_effectiveValues.find(propertyId);
    if (it != m_effectiveValues.end()) {
        return it->second;
    }
    
    // Return default value
    auto prop = DependencyProperty::GetById(propertyId);
    if (prop) {
        return prop->GetMetadata().defaultValue;
    }
    
    return std::any();
}

void Control::SetValue(DependencyProperty::Id propertyId, const std::any& value) {
    auto prop = DependencyProperty::GetById(propertyId);
    if (!prop) return;
    
    std::any oldValue = GetValue(propertyId);
    m_effectiveValues[propertyId] = value;
    
    // Check if affects layout
    const auto& metadata = prop->GetMetadata();
    if (metadata.affectsMeasure) {
        InvalidateMeasure();
    }
    if (metadata.affectsArrange) {
        InvalidateArrange();
    }
    if (metadata.affectsRender) {
        Invalidate();
    }
    
    // Call changed callback
    if (metadata.changedCallback) {
        metadata.changedCallback(this, prop->GetName());
    }
    
    OnPropertyChanged(prop->GetName());
}

void Control::ClearValue(DependencyProperty::Id propertyId) {
    m_effectiveValues.erase(propertyId);
    
    auto prop = DependencyProperty::GetById(propertyId);
    if (prop) {
        const auto& metadata = prop->GetMetadata();
        if (metadata.affectsMeasure) InvalidateMeasure();
        if (metadata.affectsArrange) InvalidateArrange();
        if (metadata.affectsRender) Invalidate();
    }
}

bool Control::HasLocalValue(DependencyProperty::Id propertyId) const {
    return m_effectiveValues.find(propertyId) != m_effectiveValues.end();
}

// ==================== Layout ====================

void Control::Measure(const Size& availableSize) {
    if (!m_needsMeasure && m_lastAvailableSize.width == availableSize.width && 
        m_lastAvailableSize.height == availableSize.height) {
        return;  // Skip if already measured with same size
    }
    
    m_lastAvailableSize = availableSize;
    m_needsMeasure = false;
    
    // Apply constraints
    Size constrainedSize = availableSize;
    if (m_width > 0) constrainedSize.width = std::min(constrainedSize.width, m_width);
    if (m_height > 0) constrainedSize.height = std::min(constrainedSize.height, m_height);
    
    // Call override
    Size desiredSize = MeasureOverride(constrainedSize);
    
    // If explicit width/height is set, use those values
    if (m_width > 0) desiredSize.width = m_width;
    if (m_height > 0) desiredSize.height = m_height;
    
    // Apply min/max constraints
    desiredSize.width = std::max(m_minWidth, std::min(desiredSize.width, m_maxWidth));
    desiredSize.height = std::max(m_minHeight, std::min(desiredSize.height, m_maxHeight));
    
    // Include margin
    desiredSize.width += m_marginLeft + m_marginRight;
    desiredSize.height += m_marginTop + m_marginBottom;
    
    m_desiredSize = desiredSize;
}

void Control::Arrange(const Rect& finalRect) {
    if (!m_needsArrange && m_renderRect.x == finalRect.x && m_renderRect.y == finalRect.y &&
        m_renderRect.width == finalRect.width && m_renderRect.height == finalRect.height) {
        return;  // Skip if already arranged
    }
    
    m_needsArrange = false;
    m_renderRect = finalRect;
    
    // Calculate size without margin
    float contentWidth = finalRect.width - m_marginLeft - m_marginRight;
    float contentHeight = finalRect.height - m_marginTop - m_marginBottom;
    
    contentWidth = std::max(0.0f, contentWidth);
    contentHeight = std::max(0.0f, contentHeight);
    
    // Call override
    Size finalSize = ArrangeOverride(Size(contentWidth, contentHeight));
    
    m_actualWidth = finalSize.width;
    m_actualHeight = finalSize.height;
    
    Invalidate();
}

Size Control::MeasureOverride(const Size& /*availableSize*/) {
    // Default: return available size or 0,0
    return Size(0, 0);
}

Size Control::ArrangeOverride(const Size& finalSize) {
    // Default: return final size
    return finalSize;
}

void Control::InvalidateMeasure() {
    m_needsMeasure = true;
    m_needsArrange = true;
    
    // Invalidate parents too
    auto parent = GetParent();
    if (parent) {
        parent->InvalidateMeasure();
    }
}

void Control::InvalidateArrange() {
    m_needsArrange = true;
}

void Control::SetActualSize(float width, float height) {
    m_actualWidth = width;
    m_actualHeight = height;
}

// ==================== Hit Testing ====================

bool Control::HitTest(const Point& point) const {
    return m_renderRect.Contains(point);
}

ControlPtr Control::HitTestPoint(const Point& point) {
    if (!GetIsVisible() || m_opacity <= 0) {
        return nullptr;
    }
    
    if (HitTest(point)) {
        return shared_from_this();
    }
    
    return nullptr;
}

// ==================== Rendering ====================

void Control::Render(IRenderContext* context) {
    if (!context) return;
    if (!GetIsVisible() || m_opacity <= 0) {
        return;
    }
    
    context->PushState();
    
    // Apply transform if not identity
    const float* matrix = m_renderTransform.GetMatrix();
    bool isIdentity = (matrix[0] == 1.0f && matrix[1] == 0.0f && matrix[2] == 0.0f &&
                       matrix[3] == 1.0f && matrix[4] == 0.0f && matrix[5] == 0.0f);
    if (!isIdentity) {
        context->MultiplyTransform(m_renderTransform);
    }
    
    // Apply opacity
    if (m_opacity < 1.0f) {
        context->PushLayer(m_opacity);
    }
    
    // Render background
    if (m_background.a > 0) {
        auto bgBrush = context->CreateSolidColorBrush(m_background);
        context->FillRectangle(m_renderRect, bgBrush.get());
    }
    
    // Call override
    RenderOverride(context);
    
    // Restore state
    if (m_opacity < 1.0f) {
        context->PopLayer();
    }
    
    context->PopState();
    
    ClearDirty();
}

void Control::RenderOverride(IRenderContext* /*context*/) {
    // Override in derived classes
}

void Control::Invalidate() {
    m_isDirty = true;
    
    // Invalidate parent (for dirty region tracking)
    auto parent = GetParent();
    if (parent) {
        parent->Invalidate();
    }
}

// ==================== Visibility ====================

void Control::SetIsVisible(bool visible) {
    if (m_isVisible != visible) {
        m_isVisible = visible;
        InvalidateMeasure();
        Invalidate();
    }
}

void Control::SetIsFocusable(bool focusable) {
    if (m_isFocusable != focusable) {
        m_isFocusable = focusable;
        
        // 注册/注销�?FocusManager
        if (m_isFocusable) {
            FocusManager::GetInstance().RegisterFocusable(this);
        } else {
            FocusManager::GetInstance().UnregisterFocusable(this);
        }
    }
}

void Control::SetOpacity(float opacity) {
    if (!std::isfinite(opacity)) return;
    opacity = std::max(0.0f, std::min(1.0f, opacity));
    if (m_opacity != opacity) {
        m_opacity = opacity;
        Invalidate();
    }
}

// ==================== Margin/Padding ====================

void Control::SetMargin(float left, float top, float right, float bottom) {
    m_marginLeft = left;
    m_marginTop = top;
    m_marginRight = right;
    m_marginBottom = bottom;
    InvalidateMeasure();
}

void Control::SetPadding(float left, float top, float right, float bottom) {
    m_paddingLeft = left;
    m_paddingTop = top;
    m_paddingRight = right;
    m_paddingBottom = bottom;
    InvalidateMeasure();
}

// ==================== Size ====================

void Control::SetWidth(float width) {
    if (m_width != width) {
        m_width = width;
        InvalidateMeasure();
    }
}

void Control::SetHeight(float height) {
    if (m_height != height) {
        m_height = height;
        InvalidateMeasure();
    }
}

// ==================== Alignment ====================

void Control::SetHorizontalAlignment(HorizontalAlignment align) {
    if (m_hAlignment != align) {
        m_hAlignment = align;
        InvalidateArrange();
    }
}

void Control::SetVerticalAlignment(VerticalAlignment align) {
    if (m_vAlignment != align) {
        m_vAlignment = align;
        InvalidateArrange();
    }
}

// ==================== Background ====================

void Control::SetBackground(const Color& color) {
    if (m_background.r != color.r || m_background.g != color.g || 
        m_background.b != color.b || m_background.a != color.a) {
        m_background = color;
        Invalidate();
    }
}

// ==================== Events ====================

void Control::RaiseClick() {
    for (auto& handler : m_clickHandlers) {
        handler(this);
    }
}

// ==================== 路由事件系统 ====================
void Control::RemoveHandler(const RoutedEvent& routedEvent) {
    size_t key = routedEvent.GetId();
    m_routedEventHandlers.erase(key);
}

void Control::RaiseEvent(const RoutedEvent& routedEvent, RoutedEventArgs& args) {
    // 调用注册的处理器
    size_t key = routedEvent.GetId();
    auto it = m_routedEventHandlers.find(key);
    if (it != m_routedEventHandlers.end()) {
        for (auto& handler : it->second) {
            if (args.Handled) break;
            handler(this, args);
        }
    }
    
    // 调用虚拟函数
    if (!args.Handled) {
        const std::string& name = routedEvent.GetName();
        
        // 鼠标事件
        if (name == "PreviewMouseDown" || name == "MouseDown") {
            OnMouseDown(static_cast<MouseEventArgs&>(args));
        } else if (name == "PreviewMouseUp" || name == "MouseUp") {
            OnMouseUp(static_cast<MouseEventArgs&>(args));
        } else if (name == "PreviewMouseMove" || name == "MouseMove") {
            OnMouseMove(static_cast<MouseEventArgs&>(args));
        }
        // 键盘事件
        else if (name == "PreviewKeyDown" || name == "KeyDown") {
            OnKeyDown(static_cast<KeyEventArgs&>(args));
        } else if (name == "PreviewKeyUp" || name == "KeyUp") {
            OnKeyUp(static_cast<KeyEventArgs&>(args));
        }
        // 焦点事件
        else if (name == "GotFocus") {
            OnGotFocus();
        } else if (name == "LostFocus") {
            OnLostFocus();
        }
    }
}

// ==================== Focus ====================

bool Control::Focus() {
    if (!m_isFocusable) return false;
    
    // 避免递归：如果已经是焦点，直接返�?
    if (FocusManager::GetInstance().GetFocusedControl() == this) {
        m_isFocused = true;
        return true;
    }
    
    // 使用 FocusManager 设置焦点
    FocusManager::GetInstance().SetFocusedControl(this);
    
    m_isFocused = true;
    Invalidate();
    return true;
}

void Control::KillFocus() {
    if (m_isFocused) {
        m_isFocused = false;
        // 只有当前焦点是本控件时才清除焦点
        if (FocusManager::GetInstance().GetFocusedControl() == this) {
            FocusManager::GetInstance().ClearFocus();
        }
    }
    Invalidate();
}

// ==================== Property Change ====================

void Control::OnPropertyChanged(const std::string& /*propertyName*/) {
    // Override in derived classes
}

// ==================== Attached Properties ====================

void Control::SetAttachedProperty(Control* control, const std::string& key, const std::any& value) {
    if (control) {
        control->m_attachedProperties[key] = value;
    }
}

std::any Control::GetAttachedProperty(Control* control, const std::string& key) {
    if (control) {
        auto it = control->m_attachedProperties.find(key);
        if (it != control->m_attachedProperties.end()) {
            return it->second;
        }
    }
    return std::any();
}

// ==================== ContentControl ====================

void ContentControl::SetContent(const ControlPtr& content) {
    if (m_content != content) {
        auto oldContent = m_content;
        
        if (oldContent) {
            oldContent->SetParent(nullptr);
        }
        
        m_content = content;
        
        if (m_content) {
            m_content->SetParent(shared_from_this());
        }
        
        InvalidateMeasure();
    }
}

ControlPtr ContentControl::GetChild(size_t index) const {
    return (index == 0) ? m_content : nullptr;
}

void ContentControl::Render(IRenderContext* context) {
    Control::Render(context);
    
    if (m_content && m_content->GetIsVisible()) {
        m_content->Render(context);
    }
}

ControlPtr ContentControl::HitTestPoint(const Point& point) {
    if (!GetIsVisible() || m_opacity <= 0) {
        return nullptr;
    }
    
    // Check content first
    if (m_content && m_content->GetIsVisible()) {
        auto hit = m_content->HitTestPoint(point);
        if (hit) {
            return hit;
        }
    }
    
    // Check self
    if (HitTest(point)) {
        return shared_from_this();
    }
    
    return nullptr;
}

// ==================== Border ====================

void Border::SetContent(const ControlPtr& content) {
    if (m_content != content) {
        auto oldContent = m_content;
        
        if (oldContent) {
            oldContent->SetParent(nullptr);
        }
        
        m_content = content;
        
        if (m_content) {
            m_content->SetParent(shared_from_this());
        }
        
        InvalidateMeasure();
    }
}

ControlPtr Border::GetChild(size_t index) const {
    return (index == 0) ? m_content : nullptr;
}

Size Border::MeasureOverride(const Size& availableSize) {
    // Account for border thickness
    Size childAvailable(
        std::max(0.0f, availableSize.width - m_borderThickness * 2),
        std::max(0.0f, availableSize.height - m_borderThickness * 2)
    );
    
    if (m_content) {
        m_content->Measure(childAvailable);
        auto childSize = m_content->GetDesiredSize();
        return Size(
            childSize.width + m_borderThickness * 2,
            childSize.height + m_borderThickness * 2
        );
    }
    
    return Size(m_borderThickness * 2, m_borderThickness * 2);
}

Size Border::ArrangeOverride(const Size& finalSize) {
    if (m_content) {
        // Content rect is relative to Border's position
        Rect childRect(
            m_renderRect.x + m_borderThickness,
            m_renderRect.y + m_borderThickness,
            finalSize.width - m_borderThickness * 2,
            finalSize.height - m_borderThickness * 2
        );
        
        m_content->Arrange(childRect);
    }
    
    return finalSize;
}

void Border::Render(IRenderContext* context) {
    // Render background
    if (m_background.a > 0) {
        auto bgBrush = context->CreateSolidColorBrush(m_background);
        if (m_cornerRadius.topLeft > 0) {
            context->FillRoundedRectangle(m_renderRect, m_cornerRadius, bgBrush.get());
        } else {
            context->FillRectangle(m_renderRect, bgBrush.get());
        }
    }
    
    // Render border
    if (m_borderThickness > 0 && m_borderBrush.a > 0) {
        auto borderBrush = context->CreateSolidColorBrush(m_borderBrush);
        if (m_cornerRadius.topLeft > 0) {
            context->DrawRoundedRectangle(m_renderRect, m_cornerRadius, borderBrush.get(), m_borderThickness);
        } else {
            context->DrawRectangle(m_renderRect, borderBrush.get(), m_borderThickness);
        }
    }
    
    // Render content
    if (m_content && m_content->GetIsVisible()) {
        m_content->Render(context);
    }
}

// ==================== ScrollViewer ====================

ScrollViewer::ScrollViewer() {
    SetHorizontalScrollBarVisibility(ScrollBarVisibility::Auto);
    SetVerticalScrollBarVisibility(ScrollBarVisibility::Auto);
}

void ScrollViewer::SetHorizontalOffset(float offset) {
    if (m_horizontalOffset != offset) {
        m_horizontalOffset = offset;
        ClampOffsets();
        Invalidate();  // Only need to redraw, not rearrange
    }
}

void ScrollViewer::SetVerticalOffset(float offset) {
    if (m_verticalOffset != offset) {
        m_verticalOffset = offset;
        ClampOffsets();
        Invalidate();  // Only need to redraw, not rearrange
    }
}

void ScrollViewer::SetHorizontalScrollBarVisibility(ScrollBarVisibility visibility) {
    if (m_hScrollVisibility != visibility) {
        m_hScrollVisibility = visibility;
        InvalidateMeasure();
    }
}

void ScrollViewer::SetVerticalScrollBarVisibility(ScrollBarVisibility visibility) {
    if (m_vScrollVisibility != visibility) {
        m_vScrollVisibility = visibility;
        InvalidateMeasure();
    }
}

void ScrollViewer::ScrollToHorizontalOffset(float offset) {
    SetHorizontalOffset(offset);
}

void ScrollViewer::ScrollToVerticalOffset(float offset) {
    SetVerticalOffset(offset);
}

void ScrollViewer::LineLeft() {
    SetHorizontalOffset(m_horizontalOffset - 16);
}

void ScrollViewer::LineRight() {
    SetHorizontalOffset(m_horizontalOffset + 16);
}

void ScrollViewer::LineUp() {
    SetVerticalOffset(m_verticalOffset - 16);
}

void ScrollViewer::LineDown() {
    SetVerticalOffset(m_verticalOffset + 16);
}

void ScrollViewer::PageLeft() {
    SetHorizontalOffset(m_horizontalOffset - m_viewport.width * 0.8f);
}

void ScrollViewer::PageRight() {
    SetHorizontalOffset(m_horizontalOffset + m_viewport.width * 0.8f);
}

void ScrollViewer::PageUp() {
    SetVerticalOffset(m_verticalOffset - m_viewport.height * 0.8f);
}

void ScrollViewer::PageDown() {
    SetVerticalOffset(m_verticalOffset + m_viewport.height * 0.8f);
}

void ScrollViewer::UpdateScrollBarVisibility() {
    // Determine if scrollbars should be shown
    switch (m_hScrollVisibility) {
        case ScrollBarVisibility::Visible:
            m_showHScroll = true;
            break;
        case ScrollBarVisibility::Hidden:
            m_showHScroll = false;
            break;
        case ScrollBarVisibility::Auto:
        default:
            m_showHScroll = m_extent.width > m_viewport.width + 0.5f;
            break;
    }
    
    switch (m_vScrollVisibility) {
        case ScrollBarVisibility::Visible:
            m_showVScroll = true;
            break;
        case ScrollBarVisibility::Hidden:
            m_showVScroll = false;
            break;
        case ScrollBarVisibility::Auto:
        default:
            m_showVScroll = m_extent.height > m_viewport.height + 0.5f;
            break;
    }
}

void ScrollViewer::ClampOffsets() {
    float maxHOffset = std::max(0.0f, m_extent.width - m_viewport.width);
    float maxVOffset = std::max(0.0f, m_extent.height - m_viewport.height);
    
    m_horizontalOffset = std::max(0.0f, std::min(m_horizontalOffset, maxHOffset));
    m_verticalOffset = std::max(0.0f, std::min(m_verticalOffset, maxVOffset));
}

Size ScrollViewer::MeasureOverride(const Size& availableSize) {
    // Calculate available size for content (reserve space for scrollbars)
    Size contentAvailable = availableSize;
    // Scroll visibility flags are used in ArrangeOverride
    (void)m_hScrollVisibility;  // Mark as used to avoid warning
    (void)m_vScrollVisibility;
    
    // Measure content with infinite size to get its desired size
    Size infiniteSize(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    
    if (m_content) {
        m_content->Measure(infiniteSize);
        m_extent = m_content->GetDesiredSize();
    } else {
        m_extent = Size();
    }
    
    // Calculate viewport size
    m_viewport = availableSize;
    
    // Update scrollbar visibility
    UpdateScrollBarVisibility();
    
    // Adjust viewport for visible scrollbars
    if (m_showVScroll) m_viewport.width -= ScrollBarThickness;
    if (m_showHScroll) m_viewport.height -= ScrollBarThickness;
    
    // Ensure viewport doesn't go negative
    m_viewport.width = std::max(0.0f, m_viewport.width);
    m_viewport.height = std::max(0.0f, m_viewport.height);
    
    // Re-check if scrollbars should be shown with adjusted viewport
    UpdateScrollBarVisibility();
    
    // Clamp offsets
    ClampOffsets();
    
    return availableSize;
}

Size ScrollViewer::ArrangeOverride(const Size& finalSize) {
    // Calculate viewport size accounting for scrollbars
    m_viewport = finalSize;
    if (m_showVScroll) m_viewport.width -= ScrollBarThickness;
    if (m_showHScroll) m_viewport.height -= ScrollBarThickness;
    
    m_viewport.width = std::max(0.0f, m_viewport.width);
    m_viewport.height = std::max(0.0f, m_viewport.height);
    
    // Arrange content at (0, 0) relative to content area
    // Scroll offset is applied during rendering via transform
    if (m_content) {
        float contentX = m_renderRect.x;
        float contentY = m_renderRect.y;
        m_content->Arrange(Rect(contentX, contentY, 
                               std::max(m_extent.width, m_viewport.width), 
                               std::max(m_extent.height, m_viewport.height)));
    }
    
    return finalSize;
}

void ScrollViewer::Render(IRenderContext* context) {
    // Render background
    if (m_background.a > 0) {
        auto bgBrush = context->CreateSolidColorBrush(m_background);
        context->FillRectangle(m_renderRect, bgBrush.get());
    }
    
    // Render content with clipping and transform
    if (m_content) {
        // Calculate viewport rect (excluding scrollbars)
        float viewportX = m_renderRect.x;
        float viewportY = m_renderRect.y;
        float viewportW = m_renderRect.width - (m_showVScroll ? ScrollBarThickness : 0);
        float viewportH = m_renderRect.height - (m_showHScroll ? ScrollBarThickness : 0);
        
        // Push clip to viewport area
        context->PushClip(Rect(viewportX, viewportY, viewportW, viewportH));
        
        // Push state for transform
        context->PushState();
        
        // Apply scroll offset transform
        Transform scrollTransform = Transform::Translation(-m_horizontalOffset, -m_verticalOffset);
        context->MultiplyTransform(scrollTransform);
        
        // Render content
        m_content->Render(context);
        
        // Restore transform
        context->PopState();
        
        // Restore clip
        context->PopClip();
    }
    
    // Render scrollbars
    RenderScrollBars(context);
}

void ScrollViewer::RenderScrollBars(IRenderContext* context) {
    // Horizontal scrollbar
    if (m_showHScroll && m_extent.width > m_viewport.width) {
        auto trackRect = GetHorizontalScrollBarTrackRect();
        auto thumbRect = GetHorizontalThumbRect();
        
        // Track
        auto trackBrush = context->CreateSolidColorBrush(Color::FromHex(0xF0F0F0));
        context->FillRectangle(trackRect, trackBrush.get());
        
        // Thumb
        auto thumbBrush = context->CreateSolidColorBrush(Color::FromHex(0xC0C0C0));
        context->FillRectangle(thumbRect, thumbBrush.get());
        
        // Border
        auto borderBrush = context->CreateSolidColorBrush(Color::FromHex(0x808080));
        context->DrawRectangle(trackRect, borderBrush.get(), 1.0f);
    }
    
    // Vertical scrollbar
    if (m_showVScroll && m_extent.height > m_viewport.height) {
        auto trackRect = GetVerticalScrollBarTrackRect();
        auto thumbRect = GetVerticalThumbRect();
        
        // Track
        auto trackBrush = context->CreateSolidColorBrush(Color::FromHex(0xF0F0F0));
        context->FillRectangle(trackRect, trackBrush.get());
        
        // Thumb
        auto thumbBrush = context->CreateSolidColorBrush(Color::FromHex(0xC0C0C0));
        context->FillRectangle(thumbRect, thumbBrush.get());
        
        // Border
        auto borderBrush = context->CreateSolidColorBrush(Color::FromHex(0x808080));
        context->DrawRectangle(trackRect, borderBrush.get(), 1.0f);
    }
}

Rect ScrollViewer::GetHorizontalScrollBarTrackRect() const {
    float y = m_renderRect.y + m_actualHeight - ScrollBarThickness;
    float width = m_showVScroll ? m_actualWidth - ScrollBarThickness : m_actualWidth;
    return Rect(m_renderRect.x, y, width, ScrollBarThickness);
}

Rect ScrollViewer::GetVerticalScrollBarTrackRect() const {
    float x = m_renderRect.x + m_actualWidth - ScrollBarThickness;
    float height = m_showHScroll ? m_actualHeight - ScrollBarThickness : m_actualHeight;
    return Rect(x, m_renderRect.y, ScrollBarThickness, height);
}

Rect ScrollViewer::GetHorizontalThumbRect() const {
    auto trackRect = GetHorizontalScrollBarTrackRect();
    
    float trackWidth = trackRect.width;
    float thumbWidth = std::max(ScrollBarThumbMinSize, 
                               trackWidth * (m_viewport.width / m_extent.width));
    float maxOffset = m_extent.width - m_viewport.width;
    float thumbPos = (maxOffset > 0) ? (m_horizontalOffset / maxOffset) * (trackWidth - thumbWidth) : 0;
    
    return Rect(trackRect.x + thumbPos, trackRect.y, thumbWidth, trackRect.height);
}

Rect ScrollViewer::GetVerticalThumbRect() const {
    auto trackRect = GetVerticalScrollBarTrackRect();
    
    float trackHeight = trackRect.height;
    float thumbHeight = std::max(ScrollBarThumbMinSize, 
                                trackHeight * (m_viewport.height / m_extent.height));
    float maxOffset = m_extent.height - m_viewport.height;
    float thumbPos = (maxOffset > 0) ? (m_verticalOffset / maxOffset) * (trackHeight - thumbHeight) : 0;
    
    return Rect(trackRect.x, trackRect.y + thumbPos, trackRect.width, thumbHeight);
}

bool ScrollViewer::HandleMouseDown(const Point& pt) {
    // Check horizontal scrollbar
    if (m_showHScroll) {
        auto thumbRect = GetHorizontalThumbRect();
        if (thumbRect.Contains(pt)) {
            m_isDraggingHThumb = true;
            m_dragStartPos = pt;
            m_dragStartOffset = m_horizontalOffset;
            return true;
        }
        
        // Click on track - page scroll
        auto trackRect = GetHorizontalScrollBarTrackRect();
        if (trackRect.Contains(pt)) {
            if (pt.x < thumbRect.x) PageLeft();
            else if (pt.x > thumbRect.x + thumbRect.width) PageRight();
            return true;
        }
    }
    
    // Check vertical scrollbar
    if (m_showVScroll) {
        auto thumbRect = GetVerticalThumbRect();
        if (thumbRect.Contains(pt)) {
            m_isDraggingVThumb = true;
            m_dragStartPos = pt;
            m_dragStartOffset = m_verticalOffset;
            return true;
        }
        
        // Click on track - page scroll
        auto trackRect = GetVerticalScrollBarTrackRect();
        if (trackRect.Contains(pt)) {
            if (pt.y < thumbRect.y) PageUp();
            else if (pt.y > thumbRect.y + thumbRect.height) PageDown();
            return true;
        }
    }
    
    return false;
}

bool ScrollViewer::HandleMouseMove(const Point& pt) {
    if (m_isDraggingHThumb) {
        auto trackRect = GetHorizontalScrollBarTrackRect();
        float trackWidth = trackRect.width;
        float thumbWidth = std::max(ScrollBarThumbMinSize, 
                                   trackWidth * (m_viewport.width / m_extent.width));
        float maxOffset = m_extent.width - m_viewport.width;
        
        float deltaX = pt.x - m_dragStartPos.x;
        float deltaOffset = (deltaX / (trackWidth - thumbWidth)) * maxOffset;
        SetHorizontalOffset(m_dragStartOffset + deltaOffset);
        return true;
    }
    
    if (m_isDraggingVThumb) {
        auto trackRect = GetVerticalScrollBarTrackRect();
        float trackHeight = trackRect.height;
        float thumbHeight = std::max(ScrollBarThumbMinSize, 
                                    trackHeight * (m_viewport.height / m_extent.height));
        float maxOffset = m_extent.height - m_viewport.height;
        
        float deltaY = pt.y - m_dragStartPos.y;
        float deltaOffset = (deltaY / (trackHeight - thumbHeight)) * maxOffset;
        SetVerticalOffset(m_dragStartOffset + deltaOffset);
        return true;
    }
    
    return false;
}

bool ScrollViewer::HandleMouseUp(const Point& /*pt*/) {
    if (m_isDraggingHThumb || m_isDraggingVThumb) {
        m_isDraggingHThumb = false;
        m_isDraggingVThumb = false;
        return true;
    }
    return false;
}

void ScrollViewer::OnMouseDown(MouseEventArgs& args) {
    luaui::rendering::Point pt(args.Position.X, args.Position.Y);
    if (HandleMouseDown(pt)) {
        args.Handled = true;
    }
}

void ScrollViewer::OnMouseMove(MouseEventArgs& args) {
    luaui::rendering::Point pt(args.Position.X, args.Position.Y);
    if (HandleMouseMove(pt)) {
        args.Handled = true;
    }
}

void ScrollViewer::OnMouseUp(MouseEventArgs& args) {
    luaui::rendering::Point pt(args.Position.X, args.Position.Y);
    if (HandleMouseUp(pt)) {
        args.Handled = true;
    }
}

void ScrollViewer::OnMouseWheel(MouseEventArgs& args) {
    // Handle vertical scrolling with mouse wheel
    if (m_showVScroll && m_extent.height > m_viewport.height) {
        float delta = args.Clicks / 120.0f * 30.0f; // Standard wheel delta = 120, scroll 30 pixels
        SetVerticalOffset(m_verticalOffset - delta);
        args.Handled = true;
    }
    // Handle horizontal scrolling if Shift is held (or if no vertical scroll)
    else if (m_showHScroll && m_extent.width > m_viewport.width) {
        float delta = args.Clicks / 120.0f * 30.0f;
        SetHorizontalOffset(m_horizontalOffset - delta);
        args.Handled = true;
    }
}

ControlPtr ScrollViewer::HitTestPoint(const Point& point) {
    if (!GetIsVisible() || m_opacity <= 0) {
        return nullptr;
    }
    
    // Check if point is within viewport (content area) or scrollbar area
    bool inViewport = false;
    bool inScrollbar = false;
    
    // Calculate viewport rect
    float viewportX = m_renderRect.x;
    float viewportY = m_renderRect.y;
    float viewportW = m_renderRect.width - (m_showVScroll ? ScrollBarThickness : 0);
    float viewportH = m_renderRect.height - (m_showHScroll ? ScrollBarThickness : 0);
    
    Rect viewportRect(viewportX, viewportY, viewportW, viewportH);
    if (viewportRect.Contains(point)) {
        inViewport = true;
    }
    
    // Check scrollbars
    if ((m_showHScroll && GetHorizontalScrollBarTrackRect().Contains(point)) ||
        (m_showVScroll && GetVerticalScrollBarTrackRect().Contains(point))) {
        inScrollbar = true;
    }
    
    // If in viewport, check content with coordinate transformation
    if (inViewport && m_content) {
        // Transform point to content coordinates (apply scroll offset)
        Point contentPoint(point.x + m_horizontalOffset, point.y + m_verticalOffset);
        auto hit = m_content->HitTestPoint(contentPoint);
        if (hit) {
            return hit;
        }
        // If content not hit but in viewport, return self
        return shared_from_this();
    }
    
    // If in scrollbar or bounds, return self
    if (inScrollbar || m_renderRect.Contains(point)) {
        return shared_from_this();
    }
    
    return nullptr;
}

// ==================== Button ====================

Button::Button() {
    SetIsFocusable(true);
    SetBackground(m_normalBackground);
    m_borderThickness = 1.0f;
    m_borderBrush = Color::FromHex(0x808080);
}

Size Button::MeasureOverride(const Size& availableSize) {
    Size size = Border::MeasureOverride(availableSize);
    
    // Minimum button size
    size.width = std::max(size.width, 80.0f);
    size.height = std::max(size.height, 32.0f);
    
    // Add padding
    size.width += 20;
    size.height += 10;
    
    return size;
}

void Button::RenderOverride(IRenderContext* /*context*/) {
    // Update background based on state
    if (m_isPressed) {
        SetBackground(m_pressedBackground);
    } else if (m_isHovered) {
        SetBackground(m_hoverBackground);
    } else {
        SetBackground(m_normalBackground);
    }
    
    // Content is rendered by ContentControl::Render
}

void Button::OnMouseEnter() {
    m_isHovered = true;
    Invalidate();
}

void Button::OnMouseLeave() {
    m_isHovered = false;
    m_isPressed = false;
    Invalidate();
}

void Button::OnMouseDown(MouseEventArgs& args) {
    m_isPressed = true;
    Focus();
    Invalidate();
    args.Handled = true;
}

void Button::OnMouseUp(MouseEventArgs& args) {
    if (m_isPressed) {
        m_isPressed = false;
        RaiseClick();
        Invalidate();
    }
    args.Handled = true;
}

void Button::Render(IRenderContext* context) {
    // Update background based on state
    if (m_isPressed) {
        m_background = m_pressedBackground;
    } else if (m_isHovered) {
        m_background = m_hoverBackground;
    } else {
        m_background = m_normalBackground;
    }
    
    // Render as Border (background, border, content)
    Border::Render(context);
}

// ==================== TextBlock ====================

TextBlock::TextBlock() {
    m_textSizeDirty = true;
}

void TextBlock::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        m_textSizeDirty = true;
        InvalidateMeasure();
    }
}

void TextBlock::UpdateTextSize(IRenderContext* context) {
    if (!m_textSizeDirty) return;
    
    auto format = context->CreateTextFormat(m_fontFamily, m_fontSize);
    m_textSize = format->MeasureText(m_text, std::numeric_limits<float>::max());
    m_textSizeDirty = false;
}

Size TextBlock::MeasureOverride(const Size& availableSize) {
    // Estimate text size based on font metrics
    // Average character width is roughly 0.6 * fontSize for most fonts
    float avgCharWidth = m_fontSize * 0.6f;
    float lineHeight = m_fontSize * 1.2f;
    
    // Count characters and estimate lines
    size_t charCount = m_text.length();
    float textWidth = charCount * avgCharWidth;
    
    // If width is constrained, calculate wrapped height
    float availableWidth = availableSize.width > 0 ? availableSize.width : textWidth;
    int numLines = std::max(1, static_cast<int>(std::ceil(textWidth / availableWidth)));
    
    // Clamp to available width
    float finalWidth = std::min(textWidth, availableWidth);
    float finalHeight = numLines * lineHeight;
    
    // Cache for rendering
    m_textSize = Size(finalWidth, finalHeight);
    m_textSizeDirty = false;
    
    return Size(finalWidth, finalHeight);
}

void TextBlock::Render(IRenderContext* context) {
    if (m_text.empty()) return;
    
    auto format = context->CreateTextFormat(m_fontFamily, m_fontSize);
    auto brush = context->CreateSolidColorBrush(m_foreground);
    
    context->DrawTextString(m_text, format.get(), m_renderRect.Position(), brush.get());
}

// ==================== TextBox ====================

TextBox::TextBox() {
    SetIsFocusable(true);
    SetBackground(Color::White());
    SetBorderBrush(m_normalBorder);
}

void TextBox::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_text = text;
        // Clamp caret position
        m_caretPosition = std::min(m_caretPosition, (int)m_text.length());
        ClearSelection();
        UpdateScrollOffset();
        Invalidate();
        if (m_textChangedHandler) {
            m_textChangedHandler(this, m_text);
        }
    }
}

void TextBox::SetCaretPosition(int pos) {
    pos = std::max(0, std::min(pos, (int)m_text.length()));
    if (m_caretPosition != pos) {
        m_caretPosition = pos;
        m_caretVisible = true;
        UpdateScrollOffset();
        Invalidate();
    }
}

void TextBox::SelectAll() {
    m_selectionStart = 0;
    m_selectionEnd = (int)m_text.length();
    Invalidate();
}

void TextBox::ClearSelection() {
    m_selectionStart = m_selectionEnd = 0;
    Invalidate();
}

void TextBox::OnGotFocus() {
    Control::OnGotFocus();
    m_caretVisible = true;
    m_caretBlinkTime = 0;
    SetBorderBrush(m_focusedBorder);
    Invalidate();
}

void TextBox::OnLostFocus() {
    Control::OnLostFocus();
    m_caretVisible = false;
    ClearSelection();
    SetBorderBrush(m_normalBorder);
    Invalidate();
}

Size TextBox::MeasureOverride(const Size& availableSize) {
    // Base measurement from Border
    Size size = Border::MeasureOverride(availableSize);
    
    // Minimum textbox size
    size.width = std::max(size.width, 100.0f);
    size.height = std::max(size.height, m_fontSize * 1.5f + 8);  // font + padding
    
    return size;
}

Size TextBox::ArrangeOverride(const Size& finalSize) {
    // Base arrangement from Border
    Size size = Border::ArrangeOverride(finalSize);
    
    // Update scroll offset for caret visibility
    UpdateScrollOffset();
    
    return size;
}

void TextBox::Render(IRenderContext* context) {
    // Update caret blink state
    UpdateCaret();
    
    // Render border and background
    Border::Render(context);
    
    // Calculate content rect (consistent with HitTestPosition)
    float contentX = m_renderRect.x + m_paddingLeft + m_borderThickness + 4;
    float contentY = m_renderRect.y + (m_renderRect.height - m_fontSize) / 2;
    float contentWidth = m_renderRect.width - m_paddingLeft - m_paddingRight - m_borderThickness * 2 - 8;
    float contentHeight = m_renderRect.height - m_borderThickness * 2;
    
    // Render text or placeholder
    std::wstring displayText = GetDisplayText();
    
    if (displayText.empty() && m_placeholder.empty() == false && !m_isFocused) {
        // Render placeholder
        auto placeholderBrush = context->CreateSolidColorBrush(m_placeholderColor);
        auto format = context->CreateTextFormat(m_fontFamily, m_fontSize);
        context->DrawTextString(m_placeholder, format.get(), 
                               Point(contentX, contentY), placeholderBrush.get());
    } else if (!displayText.empty()) {
        // Render text with scroll offset
        auto textBrush = context->CreateSolidColorBrush(m_textColor);
        auto format = context->CreateTextFormat(m_fontFamily, m_fontSize);
        
        // Use scissor rect to clip text to content area
        context->PushClip(Rect(contentX, m_renderRect.y + m_borderThickness, contentWidth, contentHeight));
        context->DrawTextString(displayText, format.get(), 
                               Point(contentX - m_scrollOffset, contentY), textBrush.get());
        context->PopClip();
        
        // Render selection highlight
        if (HasSelection() && m_isFocused) {
            // Simple selection rendering (highlight whole text for now)
            auto selectionBrush = context->CreateSolidColorBrush(Color::FromHex(0x0078D4));
            // TODO: Calculate exact selection rects
        }
    }
    
    // Render caret
    if (GetIsCaretVisible()) {
        // Calculate caret position using MeasureText for accuracy
        float caretX = contentX - m_scrollOffset;
        
        if (m_caretPosition > 0 && !displayText.empty()) {
            // Measure text up to caret position
            std::wstring textBeforeCaret = displayText.substr(0, m_caretPosition);
            auto format = context->CreateTextFormat(m_fontFamily, m_fontSize);
            Size textSize = format->MeasureText(textBeforeCaret);
            caretX = contentX + textSize.width - m_scrollOffset;
        }
        
        // Clamp to visible area
        caretX = std::max(contentX, std::min(caretX, contentX + contentWidth));
        
        auto caretBrush = context->CreateSolidColorBrush(m_textColor);
        context->FillRectangle(Rect(caretX, contentY, 1, m_fontSize), caretBrush.get());
    }
}

void TextBox::OnMouseDown(MouseEventArgs& args) {
    Focus();
    // Calculate caret position from click
    Point pt(args.Position.X, args.Position.Y);
    int pos = HitTestPosition(pt);
    SetCaretPosition(pos);
    ClearSelection();
    args.Handled = true;
}

void TextBox::OnKeyDown(KeyEventArgs& args) {
    switch (args.KeyCode) {
        case VK_LEFT:
            if (m_caretPosition > 0) {
                SetCaretPosition(m_caretPosition - 1);
            }
            args.Handled = true;
            break;
            
        case VK_RIGHT:
            if (m_caretPosition < (int)m_text.length()) {
                SetCaretPosition(m_caretPosition + 1);
            }
            args.Handled = true;
            break;
            
        case VK_HOME:
            SetCaretPosition(0);
            args.Handled = true;
            break;
            
        case VK_END:
            SetCaretPosition((int)m_text.length());
            args.Handled = true;
            break;
            
        case VK_DELETE:
            if (HasSelection()) {
                DeleteSelection();
            } else if (m_caretPosition < (int)m_text.length()) {
                m_text.erase(m_caretPosition, 1);
                SetText(m_text);
            }
            args.Handled = true;
            break;
            
        case 'A':
            if (args.Control) {
                SelectAll();
                args.Handled = true;
            }
            break;
            
        case 'C':
            if (args.Control && HasSelection()) {
                // Copy to clipboard would go here
                args.Handled = true;
            }
            break;
            
        case 'V':
            if (args.Control && !m_isReadOnly) {
                // Paste from clipboard would go here
                args.Handled = true;
            }
            break;
            
        case 'X':
            if (args.Control && HasSelection() && !m_isReadOnly) {
                // Cut to clipboard would go here
                DeleteSelection();
                args.Handled = true;
            }
            break;
    }
}

void TextBox::OnChar(wchar_t ch) {
    if (m_isReadOnly) return;
    
    // Ignore control characters (but allow backspace, tab, return)
    if (ch < 32 && ch != VK_BACK && ch != VK_TAB && ch != VK_RETURN) return;
    
    // Handle backspace
    if (ch == VK_BACK) {
        if (HasSelection()) {
            DeleteSelection();
        } else if (m_caretPosition > 0) {
            m_caretPosition--;
            m_text.erase(m_caretPosition, 1);
            SetText(m_text);
        }
        return;
    }
    
    // Check max length
    if (m_maxLength > 0 && (int)m_text.length() >= m_maxLength) {
        return;
    }
    
    // Delete selection and insert character
    if (HasSelection()) {
        DeleteSelection();
    }
    
    m_text.insert(m_caretPosition, 1, ch);
    m_caretPosition++;
    SetText(m_text);
}

void TextBox::UpdateCaret() {
    if (m_isFocused) {
        m_caretBlinkTime += 16;  // Approximate frame time
        if (m_caretBlinkTime >= CaretBlinkInterval) {
            m_caretBlinkTime = 0;
            m_caretVisible = !m_caretVisible;
            Invalidate();
        }
    }
}

void TextBox::InsertText(const std::wstring& text) {
    if (m_isReadOnly) return;
    
    // Check max length
    if (m_maxLength > 0) {
        int remaining = m_maxLength - (int)m_text.length();
        if (remaining <= 0) return;
        if ((int)text.length() > remaining) {
            m_text.insert(m_caretPosition, text.substr(0, remaining));
            m_caretPosition += remaining;
        } else {
            m_text.insert(m_caretPosition, text);
            m_caretPosition += (int)text.length();
        }
    } else {
        m_text.insert(m_caretPosition, text);
        m_caretPosition += (int)text.length();
    }
    SetText(m_text);
}

void TextBox::DeleteSelection() {
    if (!HasSelection()) return;
    
    int start = std::min(m_selectionStart, m_selectionEnd);
    int end = std::max(m_selectionStart, m_selectionEnd);
    
    m_text.erase(start, end - start);
    m_caretPosition = start;
    ClearSelection();
    SetText(m_text);
}

void TextBox::UpdateScrollOffset() {
    // Calculate caret pixel position using same logic as HitTestPosition
    float caretPixelPos = 0;
    float enWidth = m_fontSize * 0.5f;  // English/numeric width
    float cnWidth = m_fontSize * 0.95f; // Chinese character width
    
    for (int i = 0; i < m_caretPosition && i < (int)m_text.length(); i++) {
        wchar_t ch = m_text[i];
        if ((ch >= 0x4E00 && ch <= 0x9FFF) || 
            (ch >= 0x3400 && ch <= 0x4DBF) ||
            (ch >= 0xFF00 && ch <= 0xFFEF)) {
            caretPixelPos += cnWidth;
        } else {
            caretPixelPos += enWidth;
        }
    }
    
    // Use m_renderRect.width to be consistent with Render()
    float contentWidth = m_renderRect.width - m_paddingLeft - m_paddingRight - m_borderThickness * 2 - 8;
    float charWidth = (m_caretPosition > 0 && m_caretPosition <= (int)m_text.length() && 
                       ((m_text[m_caretPosition-1] >= 0x4E00 && m_text[m_caretPosition-1] <= 0x9FFF) ||
                        (m_text[m_caretPosition-1] >= 0x3400 && m_text[m_caretPosition-1] <= 0x4DBF) ||
                        (m_text[m_caretPosition-1] >= 0xFF00 && m_text[m_caretPosition-1] <= 0xFFEF))) ? cnWidth : enWidth;
    
    if (caretPixelPos < m_scrollOffset) {
        m_scrollOffset = caretPixelPos;
    } else if (caretPixelPos > m_scrollOffset + contentWidth - charWidth) {
        m_scrollOffset = caretPixelPos - contentWidth + charWidth;
    }
    
    m_scrollOffset = std::max(0.0f, m_scrollOffset);
}

int TextBox::HitTestPosition(const Point& point) {
    // contentX is where text starts rendering (accounting for padding and scroll)
    // Must match the value in Render()
    float contentX = m_renderRect.x + m_paddingLeft + m_borderThickness + 4;
    // Text is rendered at (contentX - m_scrollOffset)
    // Click position relative to text start:
    float localX = point.x - (contentX - m_scrollOffset);
    
    // Use more accurate width estimates based on Segoe UI metrics
    // Chinese characters are approximately square (width = fontSize)
    // English characters are approximately half width (width = 0.5 * fontSize)
    float x = 0;
    float enWidth = m_fontSize * 0.5f;  // English/numeric width
    float cnWidth = m_fontSize * 0.95f; // Chinese character width (slightly less than 1.0)
    
    for (size_t i = 0; i < m_text.length(); i++) {
        wchar_t ch = m_text[i];
        float charWidth;
        // CJK Unified Ideographs: 0x4E00 - 0x9FFF
        // CJK Unified Ideographs Extension A: 0x3400 - 0x4DBF
        // Fullwidth forms: 0xFF00 - 0xFFEF
        if ((ch >= 0x4E00 && ch <= 0x9FFF) || 
            (ch >= 0x3400 && ch <= 0x4DBF) ||
            (ch >= 0xFF00 && ch <= 0xFFEF)) {
            charWidth = cnWidth;
        } else {
            charWidth = enWidth;
        }
        
        // Check if click is in the first half of this character
        if (localX < x + charWidth / 2) {
            return static_cast<int>(i);
        }
        x += charWidth;
    }
    
    return static_cast<int>(m_text.length());
}

std::wstring TextBox::GetDisplayText() const {
    if (m_isPassword) {
        return std::wstring(m_text.length(), L'\u2022');  // Bullet character
    }
    return m_text;
}

// ==================== ListBoxItem ====================

ListBoxItem::ListBoxItem() {
    SetIsFocusable(false);
}

void ListBoxItem::SetContent(const std::wstring& content) {
    if (m_content != content) {
        m_content = content;
        InvalidateMeasure();
    }
}

void ListBoxItem::SetIsSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        Invalidate();
    }
}

void ListBoxItem::SetIsHovered(bool hovered) {
    if (m_isHovered != hovered) {
        m_isHovered = hovered;
        Invalidate();
    }
}

Size ListBoxItem::MeasureOverride(const Size& /*availableSize*/) {
    // Estimate text size
    float avgCharWidth = m_fontSize * 0.6f;
    float textWidth = m_content.length() * avgCharWidth;
    float textHeight = m_fontSize * 1.2f;
    
    return Size(textWidth + 16, textHeight + 8);  // padding
}

void ListBoxItem::Render(IRenderContext* context) {
    // Background
    Color bgColor;
    if (m_isSelected) {
        bgColor = m_selectedBg;
    } else if (m_isHovered) {
        bgColor = m_hoverBg;
    } else {
        bgColor = m_normalBg;
    }
    
    auto bgBrush = context->CreateSolidColorBrush(bgColor);
    context->FillRectangle(m_renderRect, bgBrush.get());
    
    // Text
    Color textColor = m_isSelected ? m_selectedText : m_normalText;
    auto textBrush = context->CreateSolidColorBrush(textColor);
    auto format = context->CreateTextFormat(L"Segoe UI", m_fontSize);
    
    Point textPos(m_renderRect.x + 8, m_renderRect.y + 4);
    context->DrawTextString(m_content, format.get(), textPos, textBrush.get());
}

// ==================== ListBox ====================

ListBox::ListBox() {
    SetIsFocusable(true);
}

void ListBox::AddItem(const std::wstring& item) {
    auto listItem = std::make_shared<ListBoxItem>();
    listItem->SetContent(item);
    AddItem(listItem);
}

void ListBox::AddItem(const std::shared_ptr<ListBoxItem>& item) {
    m_items.push_back(item);
    InvalidateMeasure();
}

void ListBox::RemoveItem(int index) {
    if (index >= 0 && index < (int)m_items.size()) {
        m_items.erase(m_items.begin() + index);
        if (m_selectedIndex == index) {
            m_selectedIndex = -1;
        } else if (m_selectedIndex > index) {
            m_selectedIndex--;
        }
        InvalidateMeasure();
    }
}

void ListBox::ClearItems() {
    m_items.clear();
    m_selectedIndex = -1;
    InvalidateMeasure();
}

std::shared_ptr<ListBoxItem> ListBox::GetItem(int index) {
    if (index >= 0 && index < (int)m_items.size()) {
        return m_items[index];
    }
    return nullptr;
}

void ListBox::SetSelectedIndex(int index) {
    if (index < -1) index = -1;
    if (index >= (int)m_items.size()) index = (int)m_items.size() - 1;
    
    if (m_selectedIndex != index) {
        m_selectedIndex = index;
        UpdateItemStates();
        Invalidate();
        
        if (m_selectionChangedHandler) {
            m_selectionChangedHandler(this, m_selectedIndex);
        }
    }
}

std::wstring ListBox::GetSelectedItem() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_items.size()) {
        return m_items[m_selectedIndex]->GetContent();
    }
    return L"";
}

void ListBox::UpdateItemStates() {
    for (size_t i = 0; i < m_items.size(); i++) {
        m_items[i]->SetIsSelected((int)i == m_selectedIndex);
    }
}

Size ListBox::MeasureOverride(const Size& availableSize) {
    // Measure all items
    float maxWidth = 0;
    float totalHeight = 0;
    
    for (auto& item : m_items) {
        item->Measure(availableSize);
        auto size = item->GetDesiredSize();
        maxWidth = std::max(maxWidth, size.width);
        totalHeight += size.height;
    }
    
    // Store item height for hit testing
    if (!m_items.empty()) {
        m_itemHeight = m_items[0]->GetDesiredSize().height;
    }
    
    // Border
    maxWidth += 2;
    totalHeight += 2;
    
    return Size(maxWidth, totalHeight);
}

Size ListBox::ArrangeOverride(const Size& finalSize) {
    float y = m_renderRect.y + 1;  // Border
    
    for (auto& item : m_items) {
        auto size = item->GetDesiredSize();
        item->Arrange(Rect(m_renderRect.x + 1, y, finalSize.width - 2, size.height));
        y += size.height;
    }
    
    return finalSize;
}

void ListBox::Render(IRenderContext* context) {
    // Border
    auto borderBrush = context->CreateSolidColorBrush(Color::FromHex(0x808080));
    context->DrawRectangle(m_renderRect, borderBrush.get(), 1.0f);
    
    // Render items
    for (auto& item : m_items) {
        item->Render(context);
    }
}

void ListBox::OnMouseDown(MouseEventArgs& args) {
    Focus();
    Point pt(args.Position.X, args.Position.Y);
    int index = HitTestItem(pt);
    if (index >= 0) {
        SetSelectedIndex(index);
    }
    args.Handled = true;
}

void ListBox::OnKeyDown(KeyEventArgs& args) {
    switch (args.KeyCode) {
        case VK_UP:
            if (m_selectedIndex > 0) {
                SetSelectedIndex(m_selectedIndex - 1);
            }
            args.Handled = true;
            break;
            
        case VK_DOWN:
            if (m_selectedIndex < (int)m_items.size() - 1) {
                SetSelectedIndex(m_selectedIndex + 1);
            }
            args.Handled = true;
            break;
            
        case VK_HOME:
            SetSelectedIndex(0);
            args.Handled = true;
            break;
            
        case VK_END:
            SetSelectedIndex((int)m_items.size() - 1);
            args.Handled = true;
            break;
    }
}

int ListBox::HitTestItem(const Point& point) {
    if (point.y < m_renderRect.y || point.y >= m_renderRect.y + m_actualHeight) {
        return -1;
    }
    
    int index = static_cast<int>((point.y - m_renderRect.y - 1) / m_itemHeight);
    if (index >= 0 && index < (int)m_items.size()) {
        return index;
    }
    return -1;
}

// ==================== 路由事件函数定义 ====================
void Control::AddHandler(const RoutedEvent& routedEvent, RoutedEventHandler handler) {
    size_t key = routedEvent.GetId();
    m_routedEventHandlers[key].push_back(handler);
}

} // namespace controls
} // namespace luaui
