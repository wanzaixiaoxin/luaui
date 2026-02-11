#pragma once
#include "Panel.h"
#include <unordered_map>
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief Canvas - 绝对定位面板（新架构）
 */
class Canvas : public Panel {
public:
    Canvas();
    
    std::string GetTypeName() const override { return "Canvas"; }
    
    // 附加属性 - 静态方法
    static void SetLeft(const std::shared_ptr<IControl>& control, float left);
    static void SetTop(const std::shared_ptr<IControl>& control, float top);
    static void SetRight(const std::shared_ptr<IControl>& control, float right);
    static void SetBottom(const std::shared_ptr<IControl>& control, float bottom);
    static float GetLeft(const std::shared_ptr<IControl>& control);
    static float GetTop(const std::shared_ptr<IControl>& control);
    static float GetRight(const std::shared_ptr<IControl>& control);
    static float GetBottom(const std::shared_ptr<IControl>& control);

protected:
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    // 使用控件的 Tag 存储位置信息，或使用外部映射
    static std::unordered_map<ControlID, rendering::Point> s_positions;
};

} // namespace controls
} // namespace luaui
