#include "Components/RenderComponent.h"
#include "Control.h"
#include "IRenderContext.h"
#include "Logger.h"

namespace luaui {
namespace components {

RenderComponent::RenderComponent(Control* owner) : Component(owner) {}

void RenderComponent::Render(rendering::IRenderContext* context) {
    if (!m_owner || !context) return;
    
    utils::Logger::TraceF("[Render] %s RenderRect: %.1f,%.1f %.1fx%.1f", 
        m_owner->GetTypeName().c_str(), m_renderRect.x, m_renderRect.y, m_renderRect.width, m_renderRect.height);
    
    // 保存状态
    context->PushState();
    
    // 应用位置变换（基于 RenderRect 的位置）
    // 使用 MultiplyTransform 累加变换，保持父级容器的变换
    rendering::Transform positionTransform = rendering::Transform::Translation(m_renderRect.x, m_renderRect.y);
    context->MultiplyTransform(positionTransform);
    
    // 执行实际渲染（使用相对于当前变换的本地坐标）
    utils::Logger::Trace("[Render] About to call RenderOverride...");
    rendering::Rect localRect(0, 0, m_renderRect.width, m_renderRect.height);
    RenderOverride(context, localRect);
    utils::Logger::Trace("[Render] RenderOverride returned");
    
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
    // 调用带本地矩形参数的版本
    RenderOverride(context, m_renderRect);
}

void RenderComponent::RenderOverride(rendering::IRenderContext* context, const rendering::Rect& localRect) {
    // 默认实现：绘制背景（使用本地坐标）
    if (m_background.a > 0) {
        auto brush = context->CreateSolidColorBrush(m_background);
        if (brush) {
            context->FillRectangle(localRect, brush.get());
        }
    }
    
    // 调用 Control 的 OnRender 方法（如果子类有自定义渲染）
    if (m_owner) {
        m_owner->OnRender(context);
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
