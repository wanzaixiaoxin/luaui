#pragma once
#include "Panel.h"
#include <unordered_map>
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief Dock 枚举
 */
enum class Dock { Left, Top, Right, Bottom };

/**
 * @brief DockPanel - 停靠布局面板（新架构）
 */
class DockPanel : public Panel {
public:
    DockPanel();
    
    std::string GetTypeName() const override { return "DockPanel"; }
    
    // LastChildFill 属性
    bool GetLastChildFill() const { return m_lastChildFill; }
    void SetLastChildFill(bool value) { m_lastChildFill = value; }
    
    // 附加属性
    static void SetDock(const std::shared_ptr<IControl>& control, Dock dock);
    static Dock GetDock(const std::shared_ptr<IControl>& control);

protected:
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    bool m_lastChildFill = true;
    static std::unordered_map<ControlID, Dock> s_dockInfo;
};

} // namespace controls
} // namespace luaui
