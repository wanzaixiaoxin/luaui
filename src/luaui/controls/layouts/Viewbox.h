#pragma once
#include "Panel.h"
#include "Image.h"
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief Viewbox - 缩放视图面板（新架构）
 */
class Viewbox : public Panel {
public:
    Viewbox();
    
    std::string GetTypeName() const override { return "Viewbox"; }
    
    Stretch GetStretch() const { return m_stretch; }
    void SetStretch(Stretch stretch);

protected:
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    Stretch m_stretch;
    rendering::Size m_childSize;
};

} // namespace controls
} // namespace luaui
