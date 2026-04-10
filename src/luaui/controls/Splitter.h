#pragma once

#include "Control.h"
#include "../rendering/Types.h"
#include "../style/ThemeKeys.h"

namespace luaui {
namespace controls {

/**
 * @brief Splitter 面板分割条
 *
 * 支持：
 * - 水平/垂直方向
 * - 拖拽调整大小
 * - Hover/Active 视觉状态 + 动画
 */
class Splitter : public luaui::Control {
public:
    Splitter();

    std::string GetTypeName() const override { return "Splitter"; }

    // 方向（true = 垂直分割条，左右分割面板）
    bool GetIsVertical() const { return m_isVertical; }
    void SetIsVertical(bool vertical);

    // 厚度
    float GetThickness() const { return m_thickness; }
    void SetThickness(float t);

    // 最小尺寸约束
    float GetMinBefore() const { return m_minBefore; }
    void SetMinBefore(float size) { m_minBefore = size; }
    float GetMinAfter() const { return m_minAfter; }
    void SetMinAfter(float size) { m_minAfter = size; }

    // 事件
    Delegate<Splitter*, float> PositionChanged;

protected:
    void InitializeComponents() override;
    void ApplyTheme() override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    void OnRender(rendering::IRenderContext* context) override;

    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnMouseDown(MouseEventArgs& args) override;
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseUp(MouseEventArgs& args) override;

private:
    rendering::Color GetTargetBgColor() const;
    void AnimateBgTo(const rendering::Color& target, float durationMs);
    void DrawGripDots(rendering::IRenderContext* context, const rendering::Rect& rect);

    bool m_isVertical = true;
    float m_thickness = 5.0f;
    float m_minBefore = 100.0f;
    float m_minAfter = 100.0f;
    bool m_isDragging = false;
    bool m_isHovered = false;
    float m_dragStartPos = 0;

    // 颜色
    rendering::Color m_bgColor;
    rendering::Color m_hoverBg;
    rendering::Color m_activeBg;
    rendering::Color m_gripColor;
    rendering::Color m_animBg;
};

} // namespace controls
} // namespace luaui
