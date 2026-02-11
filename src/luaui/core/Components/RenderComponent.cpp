#include "Components/RenderComponent.h"
#include "Control.h"
#include "IRenderContext.h"

namespace luaui {
namespace components {

RenderComponent::RenderComponent(Control* owner) : Component(owner) {}

void RenderComponent::Render(rendering::IRenderContext* context) {
    if (!m_owner || !context) return;
    
    // 保存状态
    context->PushState();
    
    // 应用变换和透明度
    context->SetTransform(m_transform);
    if (m_opacity < 1.0f) {
        context->SetOpacity(m_opacity);
    }
    
    // 执行实际渲染
    RenderOverride(context);
    
    // 恢复状态
    context->PopState();
    
    m_isDirty = false;
}

void RenderComponent::SetBackground(const rendering::Color& color) {
    m_background = color;
    Invalidate();
}

void RenderComponent::SetOpacity(float opacity) {
    m_opacity = opacity;
    Invalidate();
}

void RenderComponent::SetRenderTransform(const rendering::Transform& transform) {
    m_transform = transform;
    Invalidate();
}

void RenderComponent::Invalidate() {
    m_isDirty = true;
    // TODO: 通知窗口需要重绘
}

void RenderComponent::RenderOverride(rendering::IRenderContext* context) {
    // 默认实现：绘制背景
    if (m_background.a > 0) {
        context->FillRectangle(m_renderRect, nullptr); // TODO: 需要画刷
    }
}

void RenderComponent::SetActualSize(float width, float height) {
    m_actualWidth = width;
    m_actualHeight = height;
    m_renderRect.width = width;
    m_renderRect.height = height;
}

} // namespace components
} // namespace luaui
