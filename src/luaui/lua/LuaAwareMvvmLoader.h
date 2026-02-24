#pragma once

#include "mvvm/IBindable.h"
#include "Control.h"
#include <string>
#include <memory>
#include <any>

extern "C" {
#include <lua.h>
}

namespace luaui { namespace mvvm { class MvvmXmlLoader; } }

namespace luaui {
namespace lua {

// ============================================================================
// LuaPropertyNotifier - 桥接 Lua 属性变更到 C++ 绑定引擎
// ============================================================================

class LuaPropertyNotifier : public mvvm::INotifyPropertyChanged {
public:
    LuaPropertyNotifier(lua_State* L, const std::string& viewModelName);
    
    // INotifyPropertyChanged 实现
    void SubscribePropertyChanged(PropertyChangedHandler handler) override;
    void UnsubscribePropertyChanged(PropertyChangedHandler handler) override;
    void NotifyPropertyChanged(const std::string& propertyName) override;
    std::any GetPropertyValue(const std::string& name) const override;
    void SetPropertyValue(const std::string& name, const std::any& value) override;
    
    // 调用 Lua 函数（用于 Command）
    bool CallFunction(const std::string& name);
    
    // 检查字段是否存在
    bool HasProperty(const std::string& name) const;
    bool HasFunction(const std::string& name) const;

private:
    lua_State* m_L = nullptr;
    std::string m_viewModelName;
    std::vector<PropertyChangedHandler> m_handlers;
    
    void PushViewModel() const;
};

// ============================================================================
// LuaAwareMvvmLoader - 原生支持 Lua ViewModel 的 MVVM 加载器
// ============================================================================

class LuaAwareMvvmLoader {
public:
    LuaAwareMvvmLoader();
    ~LuaAwareMvvmLoader();

    void SetLuaState(lua_State* L);
    void SetViewModelName(const std::string& name);

    std::shared_ptr<luaui::Control> Load(const std::string& filePath);

    std::shared_ptr<LuaPropertyNotifier> GetNotifier() const { return m_notifier; }

private:
    lua_State* m_L = nullptr;
    std::string m_viewModelName = "ViewModelInstance";
    std::shared_ptr<LuaPropertyNotifier> m_notifier;
    std::shared_ptr<mvvm::MvvmXmlLoader> m_baseLoader;
    
    void AutoBindCommands();
    void RegisterLuaCommand(const std::string& commandName);
};

} // namespace lua
} // namespace luaui
