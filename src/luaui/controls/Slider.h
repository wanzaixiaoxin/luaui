#pragma once

#include "Control.h"
#include <string>

namespace luaui {
namespace controls {

/**
 * @brief Slider 滑块控件（新架构）
 */
class Slider : public Control {
public:
    Slider();
    
    std::string GetTypeName() const override { return "Slider"; }
    
    // 值属性
    double GetValue() const { return m_value; }
    void SetValue(double value);
    
    double GetMinimum() const { return m_minimum; }
    void SetMinimum(double min);
    
    double GetMaximum() const { return m_maximum; }
    void SetMaximum(double max);
    
    double GetSmallChange() const { return m_smallChange; }
    void SetSmallChange(double change) { m_smallChange = change; }
    
    double GetLargeChange() const { return m_largeChange; }
    void SetLargeChange(double change) { m_largeChange = change; }
    
    bool GetIsSnapToTick() const { return m_isSnapToTick; }
    void SetIsSnapToTick(bool snap) { m_isSnapToTick = snap; }
    
    // 方向
    bool GetIsVertical() const { return m_isVertical; }
    void SetIsVertical(bool vertical);
    
    // 事件
    luaui::Delegate<Slider*, double> ValueChanged;

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;

private:
    void ClampValue();
    
    double m_value = 0.0;
    double m_minimum = 0.0;
    double m_maximum = 100.0;
    double m_smallChange = 1.0;
    double m_largeChange = 10.0;
    bool m_isSnapToTick = false;
    bool m_isVertical = false;
    bool m_isHovered = false;
};

/**
 * @brief ProgressBar 进度条（新架构）
 */
class ProgressBar : public Control {
public:
    ProgressBar();
    
    std::string GetTypeName() const override { return "ProgressBar"; }
    
    // 值属性
    double GetValue() const { return m_value; }
    void SetValue(double value);
    
    double GetMinimum() const { return m_minimum; }
    void SetMinimum(double min);
    
    double GetMaximum() const { return m_maximum; }
    void SetMaximum(double max);
    
    bool GetIsIndeterminate() const { return m_isIndeterminate; }
    void SetIsIndeterminate(bool indeterminate);
    
    bool GetIsVertical() const { return m_isVertical; }
    void SetIsVertical(bool vertical);

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;

private:
    double m_value = 0.0;
    double m_minimum = 0.0;
    double m_maximum = 100.0;
    bool m_isIndeterminate = false;
    bool m_isVertical = false;
};

} // namespace controls
} // namespace luaui
