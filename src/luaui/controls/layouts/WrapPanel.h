#pragma once
#include "Panel.h"
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief WrapPanel - 自动换行面板（新架构）
 */
class WrapPanel : public Panel {
public:
    enum class Orientation { Horizontal, Vertical };
    
    WrapPanel();
    
    std::string GetTypeName() const override { return "WrapPanel"; }
    
    // 方向属性
    Orientation GetOrientation() const { return m_orientation; }
    void SetOrientation(Orientation orient) { m_orientation = orient; }
    
    // ItemWidth/ItemHeight 用于统一子项大小
    float GetItemWidth() const { return m_itemWidth; }
    void SetItemWidth(float width) { m_itemWidth = width; }
    
    float GetItemHeight() const { return m_itemHeight; }
    void SetItemHeight(float height) { m_itemHeight = height; }

protected:
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    Orientation m_orientation = Orientation::Horizontal;
    float m_itemWidth = 0;   // 0 表示使用子项期望宽度
    float m_itemHeight = 0;  // 0 表示使用子项期望高度
};

} // namespace controls
} // namespace luaui
