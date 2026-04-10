#include "SideBar.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Interfaces/IRenderable.h"
#include "IRenderContext.h"
#include "Theme.h"
#include "ThemeKeys.h"

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
    width = std::clamp(width, m_minWidth, m_maxWidth);
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
    if (m_content && !m_isCollapsed) {
        if (auto* layoutable = m_content->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(w, availableSize.height - m_headerHeight);
            layoutable->Measure(constraint);
        }
    }
    return rendering::Size(w, availableSize.height);
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

    auto rect = render->GetRenderRect();

    // background
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }

    // right border
    auto borderBrush = context->CreateSolidColorBrush(m_sbBorderColor);
    if (borderBrush) {
        context->DrawLine(
            rendering::Point(rect.x + rect.width - 1, rect.y),
            rendering::Point(rect.x + rect.width - 1, rect.y + rect.height),
            borderBrush.get(), 1.0f);
    }

    // header area
    rendering::Rect headerRect(rect.x, rect.y, rect.width, m_headerHeight);
    DrawHeader(context, headerRect);

    // collapse button
    if (!m_isCollapsed) {
        float btnSize = 16.0f;
        float btnX = rect.x + rect.width - btnSize - 8;
        float btnY = rect.y + (m_headerHeight - btnSize) / 2;
        DrawCollapseButton(context, rendering::Rect(btnX, btnY, btnSize, btnSize));
    }

    // title text
    if (!m_isCollapsed && !m_title.empty()) {
        auto textBrush = context->CreateSolidColorBrush(m_headerText);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", 14.0f);
        if (textBrush && textFormat) {
            float textY = rect.y + (m_headerHeight - 14.0f) / 2;
            context->DrawTextString(m_title, textFormat.get(),
                                    rendering::Point(rect.x + 12, textY), textBrush.get());
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
    auto rect = render->GetRenderRect();

    float btnSize = 16.0f;
    float btnX = rect.x + rect.width - btnSize - 8;
    float btnY = rect.y + (m_headerHeight - btnSize) / 2;

    return x >= btnX && x <= btnX + btnSize &&
           y >= btnY && y <= btnY + btnSize;
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
