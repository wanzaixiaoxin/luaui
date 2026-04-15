#pragma once
#include "Panel.h"
#include <unordered_map>
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief Canvas - absolute positioning panel
 */
class Canvas : public Panel {
public:
    Canvas();
    
    std::string GetTypeName() const override { return "Canvas"; }
    
    // Attached properties - static methods
    static void SetLeft(const std::shared_ptr<IControl>& control, float left);
    static void SetTop(const std::shared_ptr<IControl>& control, float top);
    static void SetRight(const std::shared_ptr<IControl>& control, float right);
    static void SetBottom(const std::shared_ptr<IControl>& control, float bottom);
    static void SetZIndex(const std::shared_ptr<IControl>& control, int zIndex);
    static float GetLeft(const std::shared_ptr<IControl>& control);
    static float GetTop(const std::shared_ptr<IControl>& control);
    static float GetRight(const std::shared_ptr<IControl>& control);
    static float GetBottom(const std::shared_ptr<IControl>& control);
    static int GetZIndex(const std::shared_ptr<IControl>& control);

protected:
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;

private:
    // Position info for each control
    struct PositionInfo {
        float left = 0;
        float top = 0;
        float right = -1;  // -1 means not set
        float bottom = -1; // -1 means not set
        int zIndex = 0;
    };
    static std::unordered_map<ControlID, PositionInfo> s_positionInfo;
};

} // namespace controls
} // namespace luaui
