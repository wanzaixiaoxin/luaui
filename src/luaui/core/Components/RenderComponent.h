#pragma once

#include "Components/Component.h"
#include "Interfaces/IRenderable.h"

namespace luaui {

class Control;

namespace rendering {
    class IRenderContext;
}

namespace components {

using namespace luaui::interfaces;

/**
 * @brief 渲染组件
 * 
 * 将渲染相关状态和行为从 Control 中分离
 * 符合 SRP：只负责渲染状态和绘制
 */
class RenderComponent : public Component, public IRenderable {
public:
    RenderComponent(Control* owner);
    
    // ========== IRenderable 实现 ==========
    void Render(rendering::IRenderContext* context) override;
    
    const rendering::Rect& GetRenderRect() const override { return m_renderRect; }
    rendering::Rect& GetRenderRect() override { return m_renderRect; }
    
    rendering::Color GetBackground() const override { return m_background; }
    void SetBackground(const rendering::Color& color) override;
    
    float GetOpacity() const override { return m_opacity; }
    void SetOpacity(float opacity) override;
    
    const rendering::Transform& GetRenderTransform() const override { return m_transform; }
    void SetRenderTransform(const rendering::Transform& transform) override;

    bool IsDirty() const override { return m_isDirty; }
    void Invalidate() override;
    void ClearDirtyFlag() override { m_isDirty = false; }

    // ========== 扩展点 ==========
    virtual void RenderOverride(rendering::IRenderContext* context);
    
    // ========== 辅助方法 ==========
    void SetActualSize(float width, float height);
    float GetActualWidth() const { return m_actualWidth; }
    float GetActualHeight() const { return m_actualHeight; }

private:
    rendering::Rect m_renderRect;
    rendering::Color m_background = rendering::Color::Transparent();
    float m_opacity = 1.0f;
    rendering::Transform m_transform;
    
    float m_actualWidth = 0;
    float m_actualHeight = 0;
    bool m_isDirty = true;
};

} // namespace components
} // namespace luaui
