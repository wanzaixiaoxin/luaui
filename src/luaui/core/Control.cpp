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
    // 注意：不在构造函数中调用 InitializeComponents
    // 因为虚函数在构造函数中调用不会调用到派生类版本
}

void Control::EnsureInitialized() {
    if (!m_initialized) {
        // 先设置标志，防止递归调用（InitializeComponents 中可能访问组件）
        m_initialized = true;
        InitializeComponents();
    }
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

// 组件便捷访问 - 带缓存优化
void Control::InvalidateComponentCache() {
    m_cachedLayout = nullptr;
    m_cachedRender = nullptr;
    m_cachedInput = nullptr;
}

components::LayoutComponent* Control::GetLayout() {
    // 确保已初始化（延迟初始化）
    EnsureInitialized();
    
    // 检查缓存
    if (m_cachedLayout) {
        return m_cachedLayout;
    }
    
    // 首先尝试精确匹配
    if (auto* layout = m_components.GetComponent<components::LayoutComponent>()) {
        m_cachedLayout = layout;
        return layout;
    }
    
    // 如果没有找到，尝试查找派生类
    for (auto& [type, comp] : m_components.GetComponents()) {
        if (auto* layout = dynamic_cast<components::LayoutComponent*>(comp.get())) {
            m_cachedLayout = layout;
            return layout;
        }
    }
    return nullptr;
}

components::RenderComponent* Control::GetRender() {
    // 确保已初始化（延迟初始化）
    EnsureInitialized();
    
    // 检查缓存
    if (m_cachedRender) {
        return m_cachedRender;
    }
    
    // 首先尝试精确匹配
    if (auto* render = m_components.GetComponent<components::RenderComponent>()) {
        m_cachedRender = render;
        return render;
    }
    
    // 如果没有找到，尝试查找派生类
    for (auto& [type, comp] : m_components.GetComponents()) {
        if (auto* render = dynamic_cast<components::RenderComponent*>(comp.get())) {
            m_cachedRender = render;
            return render;
        }
    }
    return nullptr;
}

components::InputComponent* Control::GetInput() {
    // 确保已初始化（延迟初始化）
    EnsureInitialized();
    
    // 检查缓存
    if (m_cachedInput) {
        return m_cachedInput;
    }
    
    if (auto* input = m_components.GetComponent<components::InputComponent>()) {
        m_cachedInput = input;
        return input;
    }
    return nullptr;
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
