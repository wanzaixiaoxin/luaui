#pragma once

#include "Types.h"
#include <memory>

namespace luaui {
namespace rendering {
    class IRenderContext;
}

namespace interfaces {

/**
 * @brief 可渲染对象接口
 * 
 * 符合 ISP 原则：只包含渲染相关方法
 * 只有需要渲染的控件才实现此接口
 */
class IRenderable {
public:
    virtual ~IRenderable() = default;

    // ========== 渲染 ==========
    virtual void Render(rendering::IRenderContext* context) = 0;
    
    // ========== 渲染状态 ==========
    virtual const rendering::Rect& GetRenderRect() const = 0;
    virtual rendering::Rect& GetRenderRect() = 0;
    
    virtual rendering::Color GetBackground() const = 0;
    virtual void SetBackground(const rendering::Color& color) = 0;
    
    virtual float GetOpacity() const = 0;
    virtual void SetOpacity(float opacity) = 0;
    
    virtual const rendering::Transform& GetRenderTransform() const = 0;
    virtual void SetRenderTransform(const rendering::Transform& transform) = 0;

    // ========== 脏状态 ==========
    virtual bool IsDirty() const = 0;
    virtual void Invalidate() = 0;
    virtual void ClearDirtyFlag() = 0;
};

/**
 * @brief 可渲染容器（用于 Panel）
 */
class IRenderContainer {
public:
    virtual ~IRenderContainer() = default;
    
    // 子控件渲染顺序控制
    virtual void RenderChildren(rendering::IRenderContext* context) = 0;
};

} // namespace interfaces
} // namespace luaui
