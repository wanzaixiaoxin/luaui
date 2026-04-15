#include "SideBar.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Interfaces/IRenderable.h"
#include "IRenderContext.h"
#include "Theme.h"
#include "ThemeKeys.h"
#include "Logger.h"

namespace luaui {
namespace controls {

SideBar::SideBar() {}

void SideBar::InitializeComponents() {
    Panel::InitializeComponents();
    if (auto* layout = GetLayout()) {
        layout->SetWidth(m_width);
        layout->SetMinWidth(m_collapsedWidth);
    }
}

void SideBar::SetTitle(const std::wstring& title) {
    if (m_title != title) {
        m_title = title;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void SideBar::SetSideBarWidth(float width) {
    // 使用 LayoutComponent 的 MinWidth/MaxWidth
    float minWidth = m_minWidth;
    float maxWidth = m_maxWidth;
    if (auto* layout = GetLayout()) {
        if (layout->GetMinWidth() > 0.0f) {
            minWidth = layout->GetMinWidth();
        }
        if (layout->GetMaxWidth() > 0.0f && layout->GetMaxWidth() < 99990.0f) {
            maxWidth = layout->GetMaxWidth();
        }
    }
    
    width = std::clamp(width, minWidth, maxWidth);
    if (m_width != width) {
        m_width = width;
        if (!m_isCollapsed) {
            if (auto* layout = GetLayout()) {
                layout->SetWidth(width);
                layout->InvalidateMeasure();
            }
        }
    }
}

void SideBar::SetIsCollapsed(bool collapsed) {
    if (m_isCollapsed != collapsed) {
        m_isCollapsed = collapsed;
        if (auto* layout = GetLayout()) {
            layout->SetWidth(collapsed ? m_collapsedWidth : m_width);
            layout->InvalidateMeasure();
        }
        if (m_content) {
            m_content->SetIsVisible(!collapsed);
        }
        CollapsedChanged.Invoke(this, collapsed);
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void SideBar::SetContent(const std::shared_ptr<Control>& content) {
    if (m_content) {
        RemoveChild(m_content);
    }
    m_content = content;
    if (content) {
        content->SetIsVisible(!m_isCollapsed);
        AddChild(content);
    }
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void SideBar::ApplyTheme() {
    auto& t = Theme::GetCurrent();
    using namespace theme;
    m_bgColor = t.GetColor(kSideBarBg);
    m_sbBorderColor = t.GetColor(kSideBarBorder);
    m_headerBg = t.GetColor(kSideBarHeaderBg);
    m_headerText = t.GetColor(kSideBarHeaderText);
    m_collapseBtn = t.GetColor(kSideBarCollapseBtn);
    m_collapseBtnHover = t.GetColor(kSideBarCollapseBtnHover);
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

rendering::Size SideBar::OnMeasureChildren(const rendering::Size& availableSize) {
    float w = m_isCollapsed ? m_collapsedWidth : m_width;
    float h = m_headerHeight;
    if (m_content && !m_isCollapsed) {
        if (auto* layoutable = m_content->AsLayoutable()) {
            float availH = std::min(availableSize.height, 10000.0f) - m_headerHeight;
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(w, availH);
            layoutable->Measure(constraint);
            h += layoutable->GetDesiredSize().height;
        }
    }
    return rendering::Size(w, h);
}

rendering::Size SideBar::OnArrangeChildren(const rendering::Size& finalSize) {
    float w = m_isCollapsed ? m_collapsedWidth : m_width;
    if (m_content && !m_isCollapsed) {
        if (auto* layoutable = m_content->AsLayoutable()) {
            layoutable->Arrange(rendering::Rect(0, m_headerHeight, w, finalSize.height - m_headerHeight));
        }
    }
    return rendering::Size(w, finalSize.height);
}

void SideBar::DrawHeader(rendering::IRenderContext* context, const rendering::Rect& bounds) {
    auto hdrBrush = context->CreateSolidColorBrush(m_headerBg);
    if (hdrBrush) {
        context->FillRectangle(bounds, hdrBrush.get());
    }
}

void SideBar::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;

    auto* render = GetRender();
    if (!render) return;

    float w = render->GetRenderRect().width;
    float h = render->GetRenderRect().height;

    // background (local coordinates)
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    if (bgBrush) {
        context->FillRectangle(rendering::Rect(0, 0, w, h), bgBrush.get());
    }

    // right border (local coordinates)
    auto borderBrush = context->CreateSolidColorBrush(m_sbBorderColor);
    if (borderBrush) {
        context->DrawLine(
            rendering::Point(w - 1, 0),
            rendering::Point(w - 1, h),
            borderBrush.get(), 1.0f);
    }

    // header area (local coordinates)
    DrawHeader(context, rendering::Rect(0, 0, w, m_headerHeight));

    // collapse button (local coordinates)
    if (!m_isCollapsed) {
        float btnSize = 16.0f;
        float btnX = w - btnSize - 8;
        float btnY = (m_headerHeight - btnSize) / 2;
        DrawCollapseButton(context, rendering::Rect(btnX, btnY, btnSize, btnSize));
    }

    // title text (local coordinates)
    if (!m_isCollapsed && !m_title.empty()) {
        auto textBrush = context->CreateSolidColorBrush(m_headerText);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", 14.0f);
        if (textBrush && textFormat) {
            float textY = (m_headerHeight - 14.0f) / 2;
            context->DrawTextString(m_title, textFormat.get(),
                                    rendering::Point(12, textY), textBrush.get());
        }
    }

    // content
    if (m_content && !m_isCollapsed) {
        if (auto* contentRenderable = m_content->AsRenderable()) {
            contentRenderable->Render(context);
        }
    }
}

void SideBar::DrawCollapseButton(rendering::IRenderContext* context, const rendering::Rect& btnRect) {
    auto brush = context->CreateSolidColorBrush(
        m_isHeaderHovered ? m_collapseBtnHover : m_collapseBtn);
    if (!brush) return;

    float centerX = btnRect.x + btnRect.width / 2;
    float centerY = btnRect.y + btnRect.height / 2;

    context->DrawLine(rendering::Point(centerX + 3, centerY - 4),
                      rendering::Point(centerX - 3, centerY),
                      brush.get(), 1.5f);
    context->DrawLine(rendering::Point(centerX - 3, centerY),
                      rendering::Point(centerX + 3, centerY + 4),
                      brush.get(), 1.5f);
}

bool SideBar::HitTestCollapseButton(float x, float y) {
    auto* render = GetRender();
    if (!render) return false;

    rendering::Rect rect = render->GetRenderRect();
    float w = rect.width;

    float btnSize = 16.0f;
    float btnX = rect.x + w - btnSize - 8;
    float btnY = rect.y + (m_headerHeight - btnSize) / 2;

    return x >= btnX && x <= btnX + btnSize &&
           y >= btnY && y <= btnY + btnSize;
}

void SideBar::OnMouseMove(MouseEventArgs& args) {
    bool wasHovered = m_isHeaderHovered;
    m_isHeaderHovered = HitTestCollapseButton(args.x, args.y);
    
    if (wasHovered != m_isHeaderHovered) {
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void SideBar::OnMouseDown(MouseEventArgs& args) {
    if (HitTestCollapseButton(args.x, args.y)) {
        ToggleCollapsed();
        args.Handled = true;
        return;
    }
}

} // namespace controls
} // namespace luaui
