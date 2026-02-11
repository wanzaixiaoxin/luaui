#pragma once

#include "Types.h"
#include <memory>

namespace luaui {
namespace interfaces {

// 布局约束定义
struct LayoutConstraint {
    rendering::Size available;
    float minWidth = 0;
    float maxWidth = std::numeric_limits<float>::infinity();
    float minHeight = 0;
    float maxHeight = std::numeric_limits<float>::infinity();
};

/**
 * @brief 可布局对象接口
 * 
 * 符合 ISP 原则：只包含布局相关方法
 * 只有参与布局的控件才需要实现此接口
 */
class ILayoutable {
public:
    virtual ~ILayoutable() = default;

    // ========== 测量与排列 ==========
    virtual rendering::Size Measure(const LayoutConstraint& constraint) = 0;
    virtual void Arrange(const rendering::Rect& finalRect) = 0;
    
    // ========== 尺寸查询 ==========
    virtual rendering::Size GetDesiredSize() const = 0;
    
    virtual float GetWidth() const = 0;
    virtual float GetHeight() const = 0;
    virtual void SetWidth(float width) = 0;
    virtual void SetHeight(float height) = 0;
    
    virtual float GetMinWidth() const = 0;
    virtual float GetMinHeight() const = 0;
    virtual void SetMinWidth(float value) = 0;
    virtual void SetMinHeight(float value) = 0;
    
    virtual float GetMaxWidth() const = 0;
    virtual float GetMaxHeight() const = 0;
    virtual void SetMaxWidth(float value) = 0;
    virtual void SetMaxHeight(float value) = 0;

    // ========== 边距 ==========
    virtual float GetMarginLeft() const = 0;
    virtual float GetMarginTop() const = 0;
    virtual float GetMarginRight() const = 0;
    virtual float GetMarginBottom() const = 0;
    virtual void SetMargin(float left, float top, float right, float bottom) = 0;
    
    virtual float GetPaddingLeft() const = 0;
    virtual float GetPaddingTop() const = 0;
    virtual float GetPaddingRight() const = 0;
    virtual float GetPaddingBottom() const = 0;
    virtual void SetPadding(float left, float top, float right, float bottom) = 0;

    // ========== 对齐方式 ==========
    enum class HorizontalAlignment { Left, Center, Right, Stretch };
    enum class VerticalAlignment { Top, Center, Bottom, Stretch };
    
    virtual HorizontalAlignment GetHorizontalAlignment() const = 0;
    virtual VerticalAlignment GetVerticalAlignment() const = 0;
    virtual void SetHorizontalAlignment(HorizontalAlignment align) = 0;
    virtual void SetVerticalAlignment(VerticalAlignment align) = 0;

    // ========== 脏标记 ==========
    enum class LayoutDirty { None, Measure, Arrange, Visual };
    
    virtual LayoutDirty GetDirtyState() const = 0;
    virtual void InvalidateMeasure() = 0;
    virtual void InvalidateArrange() = 0;
};

/**
 * @brief 布局容器接口（用于 Panel）
 */
class ILayoutContainer {
public:
    virtual ~ILayoutContainer() = default;
    
    // 子控件管理
    virtual size_t GetChildCount() const = 0;
    virtual std::shared_ptr<ILayoutable> GetLayoutChild(size_t index) const = 0;
    
    // 布局计算
    virtual rendering::Size MeasureOverride(const rendering::Size& availableSize) = 0;
    virtual rendering::Size ArrangeOverride(const rendering::Size& finalSize) = 0;
};

} // namespace interfaces
} // namespace luaui
