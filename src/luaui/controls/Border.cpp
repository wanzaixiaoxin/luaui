#include "Border.h"
#include "IRenderContext.h"
#include "Interfaces/IControl.h"

namespace luaui {
namespace controls {

Border::Border() {
    // 初始化由 Panel 完成
}

void Border::InitializeComponents() {
    // 调用父类初始化
    Panel::InitializeComponents();
    
    // Border 默认有透明背景
    if (auto* render = GetRender()) {
        render->SetBackground(rendering::Color::Transparent());
    }
}

std::shared_ptr<interfaces::IControl> Border::GetChild() const {
    return m_content;
}

void Border::SetChild(const std::shared_ptr<IControl>& child) {
    if (m_content) {
        // 移除旧子控件
        RemoveChild(m_content);
    }
    
    m_content = child;
    
    if (m_content) {
        AddChild(m_content);
    }
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void Border::SetBorderThickness(float thickness) {
    if (m_borderThickness != thickness) {
        m_borderThickness = thickness;
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Border::SetBorderColor(const rendering::Color& color) {
    if (m_borderColor.r != color.r || m_borderColor.g != color.g || 
        m_borderColor.b != color.b || m_borderColor.a != color.a) {
        m_borderColor = color;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

rendering::Color Border::GetBackground() const {
    // const 方法中不能使用 GetRender()，需要直接访问
    // 暂时返回默认值
    return rendering::Color::Transparent();
}

void Border::SetBackground(const rendering::Color& color) {
    if (auto* render = GetRender()) {
        render->SetBackground(color);
    }
}

void Border::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    // 使用本地坐标 (0,0,width,height)
    rendering::Rect localRect(0, 0, render->GetRenderRect().width, render->GetRenderRect().height);
    
    // 绘制背景
    auto bg = render->GetBackground();
    if (bg.a > 0) {
        auto bgBrush = context->CreateSolidColorBrush(bg);
        if (bgBrush) {
            context->FillRectangle(localRect, bgBrush.get());
        }
    }
    
    // 绘制边框
    if (m_borderThickness > 0 && m_borderColor.a > 0) {
        auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
        if (borderBrush) {
            context->DrawRectangle(localRect, borderBrush.get(), m_borderThickness);
        }
    }
    
    // 渲染子控件
    OnRenderChildren(context);
}

rendering::Size Border::OnMeasureChildren(const rendering::Size& availableSize) {
    if (!m_content) {
        return rendering::Size(m_borderThickness * 2, m_borderThickness * 2);
    }
    
    auto* childLayout = static_cast<Control*>(m_content.get())->AsLayoutable();
    if (!childLayout) {
        return rendering::Size(m_borderThickness * 2, m_borderThickness * 2);
    }
    
    // 减去边框空间
    rendering::Size childAvailable(
        std::max(0.0f, availableSize.width - m_borderThickness * 2),
        std::max(0.0f, availableSize.height - m_borderThickness * 2)
    );
    
    interfaces::LayoutConstraint constraint;
    constraint.available = childAvailable;
    
    auto childSize = childLayout->Measure(constraint);
    
    // 加上边框空间
    return rendering::Size(
        childSize.width + m_borderThickness * 2,
        childSize.height + m_borderThickness * 2
    );
}

rendering::Size Border::OnArrangeChildren(const rendering::Size& finalSize) {
    if (!m_content) {
        return finalSize;
    }
    
    auto* childLayout = static_cast<Control*>(m_content.get())->AsLayoutable();
    if (!childLayout) {
        return finalSize;
    }
    
    // 为子控件分配空间（减去边框）
    childLayout->Arrange(rendering::Rect(
        m_borderThickness,
        m_borderThickness,
        finalSize.width - m_borderThickness * 2,
        finalSize.height - m_borderThickness * 2
    ));
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
