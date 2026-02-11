#include "Control.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Dispatcher.h"

namespace luaui {

std::atomic<ControlID> Control::s_idCounter{1};

Control::Control() 
    : m_id(s_idCounter.fetch_add(1, std::memory_order_relaxed))
    , m_visible(true)
    , m_enabled(true) {
    InitializeComponents();
}

Control::~Control() {
    m_components.ShutdownAll();
}

void Control::InitializeComponents() {
    // 子类可以重写此方法添加更多组件
    // 默认不添加任何组件，由子类按需添加
}

void Control::SetIsVisible(bool visible) {
    if (m_visible != visible) {
        m_visible = visible;
        PropertyChanged.Invoke(this, "IsVisible");
        
        // 如果变为可见，标记需要重绘
        if (visible) {
            if (auto* render = GetRender()) {
                render->Invalidate();
            }
        }
    }
}

void Control::SetParent(const std::shared_ptr<IControl>& parent) {
    m_parent = std::weak_ptr<IControl>(parent);
}

void Control::VerifyUIThread() const {
#ifdef _DEBUG
    if (m_dispatcher) {
        m_dispatcher->VerifyAccess();
    }
#endif
}

// 组件便捷访问
components::LayoutComponent* Control::GetLayout() {
    // 首先尝试获取 LayoutComponent
    if (auto* layout = m_components.GetComponent<components::LayoutComponent>()) {
        return layout;
    }
    // 如果没有找到，尝试查找派生类
    for (auto& [type, comp] : m_components.GetComponents()) {
        if (auto* layout = dynamic_cast<components::LayoutComponent*>(comp.get())) {
            return layout;
        }
    }
    return nullptr;
}

components::RenderComponent* Control::GetRender() {
    // 首先尝试获取 RenderComponent
    if (auto* render = m_components.GetComponent<components::RenderComponent>()) {
        return render;
    }
    // 如果没有找到，尝试获取 PanelRenderComponent（用于 Panel 及其派生类）
    // 注意：这是一个 workaround，因为 GetComponent 使用 typeid 精确匹配
    for (auto& [type, comp] : m_components.GetComponents()) {
        // 尝试 dynamic_cast 到 RenderComponent
        if (auto* render = dynamic_cast<components::RenderComponent*>(comp.get())) {
            return render;
        }
    }
    return nullptr;
}

components::InputComponent* Control::GetInput() {
    return m_components.GetComponent<components::InputComponent>();
}

// 能力接口转换
interfaces::IRenderable* Control::AsRenderable() {
    return GetRender();
}

interfaces::ILayoutable* Control::AsLayoutable() {
    return GetLayout();
}

interfaces::IInputHandler* Control::AsInputHandler() {
    return GetInput();
}

interfaces::IFocusable* Control::AsFocusable() {
    return GetInput();
}

// 默认渲染实现 - 空，子类可重写
void Control::OnRender(rendering::IRenderContext* /*context*/) {
    // 默认不执行任何操作，由子类实现自定义渲染
}

// 默认测量实现 - 返回 0,0，子类可重写
rendering::Size Control::OnMeasure(const rendering::Size& /*availableSize*/) {
    // 默认返回 0,0，由子类实现自定义测量
    return rendering::Size(0, 0);
}

} // namespace luaui
