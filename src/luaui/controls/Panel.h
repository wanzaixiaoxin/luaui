#pragma once

#include "Interfaces/IControl.h"
#include "Control.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <vector>
#include <string>
#include <memory>

namespace luaui {
namespace controls {

/**
 * @brief Panel 基类（新架构）
 * 
 * 容器控件，可以包含子控件
 */
class Panel : public Control {
public:
    Panel();
    
    std::string GetTypeName() const override { return "Panel"; }
    
    // 子控件管理
    size_t GetChildCount() const override { return m_children.size(); }
    std::shared_ptr<interfaces::IControl> GetChild(size_t index) const override;
    const std::vector<std::shared_ptr<interfaces::IControl>>& GetChildren() const { return m_children; }
    
    void AddChild(const std::shared_ptr<interfaces::IControl>& child);
    void RemoveChild(const std::shared_ptr<interfaces::IControl>& child);
    void RemoveChildAt(size_t index);
    void ClearChildren();
    void InsertChild(size_t index, const std::shared_ptr<interfaces::IControl>& child);

protected:
    void InitializeComponents() override;
    
    // 渲染子控件
    virtual void OnRenderChildren(rendering::IRenderContext* context);
    
    // 布局覆盖
    virtual rendering::Size OnMeasureChildren(const rendering::Size& availableSize);
    virtual rendering::Size OnArrangeChildren(const rendering::Size& finalSize);

protected:
    std::vector<std::shared_ptr<interfaces::IControl>> m_children;
};

/**
 * @brief StackPanel（新架构）
 * 
 * 按水平或垂直方向排列子控件
 */
class StackPanel : public Panel {
public:
    enum class Orientation { Horizontal, Vertical };
    
    StackPanel();
    
    std::string GetTypeName() const override { return "StackPanel"; }
    
    Orientation GetOrientation() const { return m_orientation; }
    void SetOrientation(Orientation orient) { m_orientation = orient; }
    
    float GetSpacing() const { return m_spacing; }
    void SetSpacing(float spacing) { m_spacing = spacing; }

protected:
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    Orientation m_orientation = Orientation::Vertical;
    float m_spacing = 0;
};

} // namespace controls
} // namespace luaui
