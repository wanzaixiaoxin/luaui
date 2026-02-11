#pragma once

#include "Interfaces/IControl.h"
#include "Interfaces/IRenderable.h"
#include "Interfaces/ILayoutable.h"
#include "Interfaces/IInputHandler.h"
#include "Components/Component.h"
#include "Delegate.h"
#include <atomic>
#include <string>

// 前向声明
namespace luaui {
namespace rendering {
    class IRenderContext;
}
namespace components {
    class RenderComponent;
}
namespace controls {
    class PanelLayoutComponent;
}
}

namespace luaui {

using ControlID = interfaces::ControlID;

namespace rendering {
    class IRenderContext;
}

class Dispatcher;

namespace components {
    class LayoutComponent;
    class RenderComponent;
    class InputComponent;
}

/**
 * @brief Control 基类
 * 
 * 使用组件模式拆分职责：
 * - LayoutComponent: 布局计算
 * - RenderComponent: 渲染
 * - InputComponent: 输入处理
 * 
 * 符合原则：
 * - SRP: 职责分散到各组件
 * - ISP: 通过接口暴露功能
 * - DIP: 依赖组件抽象
 */
class Control : public interfaces::IControl,
                public std::enable_shared_from_this<Control> {
public:
    Control();
    virtual ~Control();

    // ========== IControl 实现 ==========
    ControlID GetID() const override { return m_id; }
    std::string GetName() const override { return m_name; }
    void SetName(const std::string& name) override { m_name = name; }
    
    bool GetIsVisible() const override { return m_visible; }
    void SetIsVisible(bool visible) override;
    
    bool GetIsEnabled() const override { return m_enabled; }
    void SetIsEnabled(bool enabled) override { m_enabled = enabled; }
    
    std::shared_ptr<IControl> GetParent() const override { return m_parent.lock(); }
    void SetParent(const std::shared_ptr<IControl>& parent) override;
    
    size_t GetChildCount() const override { return 0; }
    std::shared_ptr<IControl> GetChild(size_t /*index*/) const override { return nullptr; }

    // ========== 组件访问 ==========
    components::ComponentHolder& GetComponents() { return m_components; }
    const components::ComponentHolder& GetComponents() const { return m_components; }
    
    // 便捷访问 - 实现在 cpp 文件中
    components::LayoutComponent* GetLayout();
    components::RenderComponent* GetRender();
    components::InputComponent* GetInput();

    // ========== 线程安全 ==========
    Dispatcher* GetDispatcher() const { return m_dispatcher; }
    void SetDispatcher(Dispatcher* disp) { m_dispatcher = disp; }
    void VerifyUIThread() const;

    // ========== 能力接口转换 ==========
    interfaces::IRenderable* AsRenderable() override;
    interfaces::ILayoutable* AsLayoutable() override;
    interfaces::IInputHandler* AsInputHandler() override;
    interfaces::IFocusable* AsFocusable() override;

    // ========== 事件（统一使用 Delegate） ==========
    Delegate<Control*> Click;
    Delegate<Control*> MouseEnter;
    Delegate<Control*> MouseLeave;
    Delegate<Control*> GotFocus;
    Delegate<Control*> LostFocus;
    Delegate<Control*, const std::string&> PropertyChanged;

protected:
    // 子类重写以添加组件
    virtual void InitializeComponents();
    
    // 渲染回调 - 子类重写以实现自定义渲染
    virtual void OnRender(rendering::IRenderContext* context);
    
    // 测量回调 - 子类重写以实现自定义测量
    virtual rendering::Size OnMeasure(const rendering::Size& availableSize);
    
    // 友元 - 组件和 Panel 需要调用 protected 方法
    friend class components::RenderComponent;
    friend class components::LayoutComponent;
    friend class controls::PanelLayoutComponent;

    // 成员变量
    ControlID m_id;
    std::string m_name;
    bool m_visible = true;
    bool m_enabled = true;
    
    std::weak_ptr<IControl> m_parent;
    Dispatcher* m_dispatcher = nullptr;
    
    components::ComponentHolder m_components;
    
    static std::atomic<ControlID> s_idCounter;
};

// 向后兼容别名
using ControlBase = Control;

} // namespace luaui
