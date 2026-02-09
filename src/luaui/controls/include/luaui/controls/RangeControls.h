#pragma once

#include "Control.h"

namespace luaui {
namespace controls {

// ==================== Slider ====================
class Slider : public Control {
public:
    Slider();
    
    std::string GetTypeName() const override { return "Slider"; }
    
    // 值范围
    double GetMinimum() const { return m_minimum; }
    void SetMinimum(double minimum);
    
    double GetMaximum() const { return m_maxumum; }
    void SetMaximum(double maximum);
    
    double GetValue() const { return m_value; }
    void SetValue(double value);
    
    // 步长
    double GetStep() const { return m_step; }
    void SetStep(double step) { m_step = step; }
    
    // 方向
    enum class Orientation { Horizontal, Vertical };
    Orientation GetOrientation() const { return m_orientation; }
    void SetOrientation(Orientation orient);
    
    // 事件
    using ValueChangedHandler = std::function<void(Slider* sender, double value)>;
    void SetValueChangedHandler(ValueChangedHandler handler) { m_valueChangedHandler = handler; }
    
    void Render(IRenderContext* context) override;
    
    // Public for external event handling
    void OnMouseDown(MouseEventArgs& args) override;
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseUp(MouseEventArgs& args) override;

protected:
    Size MeasureOverride(const Size& availableSize) override;

private:
    double m_minimum = 0.0;
    double m_maxumum = 100.0;
    double m_value = 0.0;
    double m_step = 1.0;
    Orientation m_orientation = Orientation::Horizontal;
    ValueChangedHandler m_valueChangedHandler;
    
    // 拖拽状态
    bool m_isDragging = false;
    
    // 尺寸
    static constexpr float TrackThickness = 4.0f;
    static constexpr float ThumbSize = 16.0f;
    
    // 计算 thumb 位置
    float CalculateThumbPosition() const;
    double ValueFromPosition(float position, float trackLength);
};

// ==================== ProgressBar ====================
class ProgressBar : public Control {
public:
    ProgressBar();
    
    std::string GetTypeName() const override { return "ProgressBar"; }
    
    // 进度值 (0-100)
    double GetValue() const { return m_value; }
    void SetValue(double value);
    
    // 是否显示文本
    bool GetShowText() const { return m_showText; }
    void SetShowText(bool show) { m_showText = show; Invalidate(); }
    
    // 方向
    enum class Orientation { Horizontal, Vertical };
    Orientation GetOrientation() const { return m_orientation; }
    void SetOrientation(Orientation orient) { m_orientation = orient; Invalidate(); }
    
    // 颜色
    Color GetFillColor() const { return m_fillColor; }
    void SetFillColor(const Color& color) { m_fillColor = color; Invalidate(); }
    
    void Render(IRenderContext* context) override;

protected:
    Size MeasureOverride(const Size& availableSize) override;

private:
    double m_value = 0.0;
    bool m_showText = true;
    Orientation m_orientation = Orientation::Horizontal;
    Color m_fillColor = Color::FromHex(0x0078D4);  // 默认蓝色
};

} // namespace controls
} // namespace luaui
