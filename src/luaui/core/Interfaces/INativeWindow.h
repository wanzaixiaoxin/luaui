#pragma once

#include "Types.h"
#include <string>
#include <functional>

namespace luaui {
namespace interfaces {

/**
 * @brief 原生窗口抽象接口
 * 
 * 符合 DIP 原则：Window 类依赖此接口，而非具体 Win32 API
 * 便于移植到 Linux (X11/Wayland)、macOS (Cocoa) 等平台
 */
class INativeWindow {
public:
    using Handle = void*;  // 平台相关的窗口句柄类型

    virtual ~INativeWindow() = default;

    // ========== 生命周期 ==========
    virtual bool Create(const std::wstring& title, int width, int height) = 0;
    virtual void Destroy() = 0;
    virtual void Close() = 0;
    
    // ========== 显示控制 ==========
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void Minimize() = 0;
    virtual void Maximize() = 0;
    virtual void Restore() = 0;
    
    // ========== 尺寸和位置 ==========
    virtual rendering::Size GetClientSize() const = 0;
    virtual rendering::Size GetWindowSize() const = 0;
    virtual void SetWindowSize(int width, int height) = 0;
    
    virtual rendering::Point GetPosition() const = 0;
    virtual void SetPosition(int x, int y) = 0;
    
    // ========== 标题 ==========
    virtual std::wstring GetTitle() const = 0;
    virtual void SetTitle(const std::wstring& title) = 0;

    // ========== 平台句柄（用于渲染上下文创建） ==========
    virtual Handle GetNativeHandle() const = 0;
    
    // ========== 消息循环集成 ==========
    virtual void ProcessMessages() = 0;  // 处理窗口消息（非阻塞）
    virtual int RunMessageLoop() = 0;     // 运行消息循环（阻塞）
    virtual void QuitMessageLoop(int exitCode) = 0;
    
    // ========== 重绘请求 ==========
    virtual void Invalidate() = 0;
    virtual void InvalidateRect(const rendering::Rect& rect) = 0;
    
    // ========== 回调设置 ==========
    using ResizeCallback = std::function<void(int width, int height)>;
    using CloseCallback = std::function<bool()>;  // 返回 false 阻止关闭
    using PaintCallback = std::function<void()>;
    
    virtual void SetResizeCallback(ResizeCallback callback) = 0;
    virtual void SetCloseCallback(CloseCallback callback) = 0;
    virtual void SetPaintCallback(PaintCallback callback) = 0;
};

/**
 * @brief 原生窗口工厂
 */
class INativeWindowFactory {
public:
    virtual ~INativeWindowFactory() = default;
    virtual std::unique_ptr<INativeWindow> CreateWindow() = 0;
};

// 创建平台相关的窗口实现
std::unique_ptr<INativeWindow> CreateNativeWindow();

} // namespace interfaces
} // namespace luaui
