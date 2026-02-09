#include "Control.h"
#include "luaui/controls/FocusManager.h"
#include <algorithm>
#include <limits>
#include <cmath>

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
    // 从FocusManager注销，避免悬空指针
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
        
        // 注册/注销到 FocusManager
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
    
    // 避免递归：如果已经是焦点，直接返回
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

// ==================== Panel ====================

ControlPtr Panel::GetChild(size_t index) const {
    if (index < m_children.size()) {
        return m_children[index];
    }
    return nullptr;
}

void Panel::AddChild(const ControlPtr& child) {
    if (child) {
        child->SetParent(shared_from_this());
        m_children.push_back(child);
        InvalidateMeasure();
    }
}

void Panel::RemoveChild(const ControlPtr& child) {
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
        if (child) child->SetParent(nullptr);
    }
    m_children.clear();
    InvalidateMeasure();
}

void Panel::Render(IRenderContext* context) {
    Control::Render(context);
    
    // Render children (they use global coordinates in their m_renderRect)
    for (auto& child : m_children) {
        if (child && child->GetIsVisible()) {
            child->Render(context);
        }
    }
}

ControlPtr Panel::HitTestPoint(const Point& point) {
    if (!GetIsVisible() || m_opacity <= 0) {
        return nullptr;
    }
    
    // Test children first in reverse order (Z-order: last child is on top)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if (auto result = (*it)->HitTestPoint(point)) {
            return result;
        }
    }
    
    // Then test self
    return Control::HitTestPoint(point);
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
    float borderH = m_borderThickness * 2;
    float borderV = m_borderThickness * 2;
    
    Size childAvailable(
        std::max(0.0f, availableSize.width - borderH),
        std::max(0.0f, availableSize.height - borderV)
    );
    
    if (m_content) {
        m_content->Measure(childAvailable);
        auto childSize = m_content->GetDesiredSize();
        return Size(childSize.width + borderH, childSize.height + borderV);
    }
    
    return Size(borderH, borderV);
}

Size Border::ArrangeOverride(const Size& finalSize) {
    if (m_content) {
        float borderH = m_borderThickness * 2;
        float borderV = m_borderThickness * 2;
        
        // Content rect is relative to Border's position
        Rect childRect(
            m_renderRect.x + m_borderThickness,
            m_renderRect.y + m_borderThickness,
            std::max(0.0f, finalSize.width - borderH),
            std::max(0.0f, finalSize.height - borderV)
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

// ==================== Canvas ====================

void Canvas::SetLeft(Control* control, float left) {
    SetAttachedProperty(control, "Canvas.Left", left);
    if (control) control->InvalidateArrange();
}

void Canvas::SetTop(Control* control, float top) {
    SetAttachedProperty(control, "Canvas.Top", top);
    if (control) control->InvalidateArrange();
}

float Canvas::GetLeft(Control* control) {
    auto val = GetAttachedProperty(control, "Canvas.Left");
    return val.has_value() ? std::any_cast<float>(val) : 0.0f;
}

float Canvas::GetTop(Control* control) {
    auto val = GetAttachedProperty(control, "Canvas.Top");
    return val.has_value() ? std::any_cast<float>(val) : 0.0f;
}

Size Canvas::MeasureOverride(const Size& /*availableSize*/) {
    Size desiredSize;
    
    for (auto& child : m_children) {
        if (child) {
            child->Measure(Size(std::numeric_limits<float>::max(), std::numeric_limits<float>::max()));
            auto childSize = child->GetDesiredSize();
            
            float left = GetLeft(child.get());
            float top = GetTop(child.get());
            
            desiredSize.width = std::max(desiredSize.width, left + childSize.width);
            desiredSize.height = std::max(desiredSize.height, top + childSize.height);
        }
    }
    
    return desiredSize;
}

Size Canvas::ArrangeOverride(const Size& finalSize) {
    for (auto& child : m_children) {
        if (child) {
            float left = GetLeft(child.get());
            float top = GetTop(child.get());
            auto childSize = child->GetDesiredSize();
            
            // Arrange with absolute positioning relative to Canvas
            child->Arrange(Rect(m_renderRect.x + left, m_renderRect.y + top, 
                               childSize.width, childSize.height));
        }
    }
    
    return finalSize;
}

// ==================== StackPanel ====================

Size StackPanel::MeasureOverride(const Size& availableSize) {
    Size desiredSize;
    
    if (m_orientation == Orientation::Vertical) {
        float maxWidth = 0;
        float totalHeight = 0;
        
        for (size_t i = 0; i < m_children.size(); ++i) {
            auto& child = m_children[i];
            if (child) {
                child->Measure(Size(availableSize.width, std::numeric_limits<float>::max()));
                auto childSize = child->GetDesiredSize();
                
                maxWidth = std::max(maxWidth, childSize.width);
                totalHeight += childSize.height;
                if (i > 0) totalHeight += m_spacing;
            }
        }
        
        desiredSize = Size(maxWidth, totalHeight);
    } else {
        float totalWidth = 0;
        float maxHeight = 0;
        
        for (size_t i = 0; i < m_children.size(); ++i) {
            auto& child = m_children[i];
            if (child) {
                child->Measure(Size(std::numeric_limits<float>::max(), availableSize.height));
                auto childSize = child->GetDesiredSize();
                
                totalWidth += childSize.width;
                if (i > 0) totalWidth += m_spacing;
                maxHeight = std::max(maxHeight, childSize.height);
            }
        }
        
        desiredSize = Size(totalWidth, maxHeight);
    }
    
    return desiredSize;
}

Size StackPanel::ArrangeOverride(const Size& finalSize) {
    if (m_orientation == Orientation::Vertical) {
        float y = m_renderRect.y;
        
        for (auto& child : m_children) {
            if (child) {
                auto childSize = child->GetDesiredSize();
                child->Arrange(Rect(m_renderRect.x, y, finalSize.width, childSize.height));
                y += childSize.height + m_spacing;
            }
        }
    } else {
        float x = m_renderRect.x;
        
        for (auto& child : m_children) {
            if (child) {
                auto childSize = child->GetDesiredSize();
                child->Arrange(Rect(x, m_renderRect.y, childSize.width, finalSize.height));
                x += childSize.width + m_spacing;
            }
        }
    }
    
    return finalSize;
}

// ==================== Grid ====================

void Grid::AddRowDefinition(float height) {
    RowDefinition def;
    def.height = height;
    m_rows.push_back(def);
}

void Grid::AddColumnDefinition(float width) {
    ColumnDefinition def;
    def.width = width;
    m_columns.push_back(def);
}

void Grid::SetRow(Control* control, int row) {
    SetAttachedProperty(control, "Grid.Row", row);
    if (control) control->InvalidateArrange();
}

void Grid::SetColumn(Control* control, int column) {
    SetAttachedProperty(control, "Grid.Column", column);
    if (control) control->InvalidateArrange();
}

void Grid::SetRowSpan(Control* control, int span) {
    SetAttachedProperty(control, "Grid.RowSpan", span);
    if (control) control->InvalidateArrange();
}

void Grid::SetColumnSpan(Control* control, int span) {
    SetAttachedProperty(control, "Grid.ColumnSpan", span);
    if (control) control->InvalidateArrange();
}

Size Grid::MeasureOverride(const Size& availableSize) {
    // Simplified grid measure - just sum up children
    Size desiredSize;
    
    for (auto& child : m_children) {
        if (child) {
            child->Measure(availableSize);
            auto childSize = child->GetDesiredSize();
            desiredSize.width = std::max(desiredSize.width, childSize.width);
            desiredSize.height = std::max(desiredSize.height, childSize.height);
        }
    }
    
    return desiredSize;
}

Size Grid::ArrangeOverride(const Size& finalSize) {
    // Simplified grid arrange
    if (m_rows.empty()) AddRowDefinition(1.0f);  // Auto
    if (m_columns.empty()) AddColumnDefinition(1.0f);  // Auto
    
    float rowHeight = finalSize.height / m_rows.size();
    float colWidth = finalSize.width / m_columns.size();
    
    for (auto& child : m_children) {
        if (child) {
            auto val = GetAttachedProperty(child.get(), "Grid.Row");
            int row = val.has_value() ? std::any_cast<int>(val) : 0;
            
            val = GetAttachedProperty(child.get(), "Grid.Column");
            int col = val.has_value() ? std::any_cast<int>(val) : 0;
            
            // Use global coordinates relative to Grid's position
            child->Arrange(Rect(m_renderRect.x + col * colWidth, 
                               m_renderRect.y + row * rowHeight, 
                               colWidth, rowHeight));
        }
    }
    
    return finalSize;
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

void Button::OnMouseDown(const Point& /*point*/) {
    m_isPressed = true;
    Focus();
    Invalidate();
}

void Button::OnMouseUp(const Point& /*point*/) {
    if (m_isPressed) {
        m_isPressed = false;
        RaiseClick();
        Invalidate();
    }
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

// ==================== 路由事件函数定义 ====================
void Control::AddHandler(const RoutedEvent& routedEvent, RoutedEventHandler handler) {
    size_t key = routedEvent.GetId();
    m_routedEventHandlers[key].push_back(handler);
}

} // namespace controls
} // namespace luaui
