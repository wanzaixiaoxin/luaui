#pragma once

#include "Components/Component.h"
#include "Interfaces/ILayoutable.h"
#include <limits>

namespace luaui {

class Control;

namespace components {

using namespace luaui::interfaces;

/**
 * @brief 布局组件
 * 
 * 将布局相关状态和行为从 Control 中分离
 * 符合 SRP：只负责布局计算和状态管理
 */
class LayoutComponent : public Component, public ILayoutable {
public:
    LayoutComponent(Control* owner);
    
    // ========== ILayoutable 实现 ==========
    rendering::Size Measure(const LayoutConstraint& constraint) override;
    void Arrange(const rendering::Rect& finalRect) override;
    
    rendering::Size GetDesiredSize() const override { return m_desiredSize; }
    
    float GetWidth() const override { return m_width; }
    float GetHeight() const override { return m_height; }
    void SetWidth(float width) override;
    void SetHeight(float height) override;
    
    float GetMinWidth() const override { return m_minWidth; }
    float GetMinHeight() const override { return m_minHeight; }
    void SetMinWidth(float value) override { m_minWidth = value; }
    void SetMinHeight(float value) override { m_minHeight = value; }
    
    float GetMaxWidth() const override { return m_maxWidth; }
    float GetMaxHeight() const override { return m_maxHeight; }
    void SetMaxWidth(float value) override { m_maxWidth = value; }
    void SetMaxHeight(float value) override { m_maxHeight = value; }

    float GetMarginLeft() const override { return m_marginLeft; }
    float GetMarginTop() const override { return m_marginTop; }
    float GetMarginRight() const override { return m_marginRight; }
    float GetMarginBottom() const override { return m_marginBottom; }
    void SetMargin(float left, float top, float right, float bottom) override;
    
    float GetPaddingLeft() const override { return m_paddingLeft; }
    float GetPaddingTop() const override { return m_paddingTop; }
    float GetPaddingRight() const override { return m_paddingRight; }
    float GetPaddingBottom() const override { return m_paddingBottom; }
    void SetPadding(float left, float top, float right, float bottom) override;

    HorizontalAlignment GetHorizontalAlignment() const override { return m_hAlignment; }
    VerticalAlignment GetVerticalAlignment() const override { return m_vAlignment; }
    void SetHorizontalAlignment(HorizontalAlignment align) override;
    void SetVerticalAlignment(VerticalAlignment align) override;

    LayoutDirty GetDirtyState() const override { return m_dirty; }
    void InvalidateMeasure() override;
    void InvalidateArrange() override;

    // ========== 扩展点 ==========
    virtual rendering::Size MeasureOverride(const rendering::Size& availableSize);
    virtual rendering::Size ArrangeOverride(const rendering::Size& finalSize);
    
    // ========== 状态查询 ==========
    bool IsMeasureValid() const { return m_measureValid; }
    bool IsArrangeValid() const { return m_arrangeValid; }
    void ClearDirty() { m_dirty = LayoutDirty::None; }

private:
    // 尺寸约束
    float m_width = 0;
    float m_height = 0;
    float m_minWidth = 0;
    float m_minHeight = 0;
    float m_maxWidth = std::numeric_limits<float>::max();
    float m_maxHeight = std::numeric_limits<float>::max();
    
    // 边距
    float m_marginLeft = 0, m_marginTop = 0, m_marginRight = 0, m_marginBottom = 0;
    float m_paddingLeft = 0, m_paddingTop = 0, m_paddingRight = 0, m_paddingBottom = 0;
    
    // 对齐
    HorizontalAlignment m_hAlignment = HorizontalAlignment::Stretch;
    VerticalAlignment m_vAlignment = VerticalAlignment::Stretch;
    
    // 布局状态
    rendering::Size m_desiredSize;
    rendering::Size m_lastAvailableSize;
    LayoutConstraint m_lastConstraint;
    bool m_measureValid = false;
    bool m_arrangeValid = false;
    LayoutDirty m_dirty = LayoutDirty::Measure;
};

} // namespace components
} // namespace luaui
