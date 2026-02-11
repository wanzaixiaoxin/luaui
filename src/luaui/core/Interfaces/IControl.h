#pragma once

#include "Types.h"
#include <cstdint>
#include <memory>
#include <string>

namespace luaui {
namespace interfaces {

// Forward declarations
class IRenderable;
class ILayoutable;
class IInputHandler;
class IFocusable;
class IStyleable;

using ControlID = uint32_t;
static constexpr ControlID INVALID_CONTROL_ID = 0;

/**
 * @brief 控件核心接口
 * 
 * 定义所有控件必须实现的基本能力
 * 符合 ISP 原则：只包含最基础的标识和生命周期管理
 */
class IControl {
public:
    virtual ~IControl() = default;

    // ========== 标识 ==========
    virtual ControlID GetID() const = 0;
    virtual std::string GetTypeName() const = 0;
    virtual std::string GetName() const = 0;
    virtual void SetName(const std::string& name) = 0;

    // ========== 可见性状态 ==========
    virtual bool GetIsVisible() const = 0;
    virtual void SetIsVisible(bool visible) = 0;
    
    virtual bool GetIsEnabled() const = 0;
    virtual void SetIsEnabled(bool enabled) = 0;

    // ========== 父子关系 ==========
    virtual std::shared_ptr<IControl> GetParent() const = 0;
    virtual void SetParent(const std::shared_ptr<IControl>& parent) = 0;
    
    virtual size_t GetChildCount() const = 0;
    virtual std::shared_ptr<IControl> GetChild(size_t index) const = 0;

    // ========== 能力查询 (用于安全转换) ==========
    virtual IRenderable* AsRenderable() { return nullptr; }
    virtual ILayoutable* AsLayoutable() { return nullptr; }
    virtual IInputHandler* AsInputHandler() { return nullptr; }
    virtual IFocusable* AsFocusable() { return nullptr; }
    virtual IStyleable* AsStyleable() { return nullptr; }
    
    virtual const IRenderable* AsRenderable() const { return nullptr; }
    virtual const ILayoutable* AsLayoutable() const { return nullptr; }
    virtual const IInputHandler* AsInputHandler() const { return nullptr; }
    virtual const IFocusable* AsFocusable() const { return nullptr; }
    virtual const IStyleable* AsStyleable() const { return nullptr; }
};

using IControlPtr = std::shared_ptr<IControl>;

} // namespace interfaces
} // namespace luaui
