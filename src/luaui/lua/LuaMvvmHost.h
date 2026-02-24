#pragma once

#include "LuaSandbox.h"
#include <memory>
#include <string>
#include <windows.h>

namespace luaui {

class Window;

namespace lua {

// ============================================================================
// LuaMvvmHost - 一键启动 Lua MVVM 应用
// 
// 使用方式：
//   LuaMvvmHost host;
//   host.Initialize(hInstance, "scripts/ViewModel.lua", "layouts/MainView.xml");
//   host.Run();
// ============================================================================

class LuaMvvmHostImpl;

class LuaMvvmHost {
public:
    LuaMvvmHost();
    ~LuaMvvmHost();

    // 禁止拷贝，允许移动
    LuaMvvmHost(const LuaMvvmHost&) = delete;
    LuaMvvmHost& operator=(const LuaMvvmHost&) = delete;
    LuaMvvmHost(LuaMvvmHost&&) noexcept;
    LuaMvvmHost& operator=(LuaMvvmHost&&) noexcept;

    // 一键初始化
    // @param hInstance Windows 实例句柄
    // @param luaPath Lua ViewModel 文件路径
    // @param xmlPath XML 布局文件路径
    // @param viewModelName Lua 全局变量名（默认为 "ViewModelInstance"）
    bool Initialize(HINSTANCE hInstance,
                   const std::string& luaPath,
                   const std::string& xmlPath,
                   const std::string& viewModelName = "ViewModelInstance");

    // 运行消息循环
    int Run();

    // 获取底层对象（高级用法）
    luaui::Window* GetWindow() const;
    LuaSandbox* GetSandbox() const;

    // 关闭
    void Shutdown();

private:
    std::unique_ptr<LuaMvvmHostImpl> m_impl;
};

} // namespace lua
} // namespace luaui
