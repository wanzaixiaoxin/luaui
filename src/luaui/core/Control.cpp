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
    return m_components.GetComponent<components::LayoutComponent>();
}

components::RenderComponent* Control::GetRender() {
    return m_components.GetComponent<components::RenderComponent>();
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

} // namespace luaui
