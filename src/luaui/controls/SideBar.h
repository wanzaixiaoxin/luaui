#pragma once

#include "Panel.h"
#include "../rendering/Types.h"
#include "../style/ThemeKeys.h"
#include <memory>
#include <string>

namespace luaui {
namespace controls {

/**
 * @brief SideBar 可折叠侧边栏
 *
 * 支持：
 * - 标题栏 + 折叠/展开按钮
 * - 固定/自动折叠模式
 * - 宽度约束
 * - 内容托管
 * - 主题集成 + 动画
 */
class SideBar : public Panel {
public:
    SideBar();

    std::string GetTypeName() const override { return "SideBar"; }

    // 标题
    std::wstring GetTitle() const { return m_title; }
    void SetTitle(const std::wstring& title);

    // 宽度
    float GetSideBarWidth() const { return m_width; }
    void SetSideBarWidth(float width);
    float GetMinWidth() const { return m_minWidth; }
    void SetMinWidth(float width) { m_minWidth = width; }
    float GetMaxWidth() const { return m_maxWidth; }
    void SetMaxWidth(float width) { m_maxWidth = width; }

    // 折叠状态
    bool GetIsCollapsed() const { return m_isCollapsed; }
    void SetIsCollapsed(bool collapsed);
    void ToggleCollapsed() { SetIsCollapsed(!m_isCollapsed); }

    // 固定模式（false = 失焦自动折叠）
    bool GetIsPinned() const { return m_isPinned; }
    void SetIsPinned(bool pinned) { m_isPinned = pinned; }

    // 内容
    void SetContent(const std::shared_ptr<Control>& content);

    // 事件
    Delegate<SideBar*, bool> CollapsedChanged;

protected:
    void InitializeComponents() override;
    void ApplyTheme() override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;

    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseDown(MouseEventArgs& args) override;

private:
    void DrawHeader(rendering::IRenderContext* context, const rendering::Rect& rect);
    void DrawCollapseButton(rendering::IRenderContext* context, const rendering::Rect& rect);
    bool HitTestCollapseButton(float x, float y);

    std::wstring m_title;
    float m_width = 250.0f;
    float m_minWidth = 200.0f;
    float m_maxWidth = 400.0f;
    float m_collapsedWidth = 40.0f;
    float m_headerHeight = 32.0f;
    bool m_isCollapsed = false;
    bool m_isPinned = true;
    bool m_isHeaderHovered = false;

    std::shared_ptr<Control> m_content;

    // 颜色
    rendering::Color m_bgColor;
    rendering::Color m_sbBorderColor;
    rendering::Color m_headerBg;
    rendering::Color m_headerText;
    rendering::Color m_collapseBtn;
    rendering::Color m_collapseBtnHover;
};

} // namespace controls
} // namespace luaui
