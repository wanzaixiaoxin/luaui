#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <string>

namespace luaui {
namespace controls {

// Forward declarations
class Control;
using ControlPtr = std::shared_ptr<Control>;

// ==================== 事件参数基类 ====================
class RoutedEventArgs {
public:
    RoutedEventArgs() = default;
    virtual ~RoutedEventArgs() = default;

    // 事件是否已处理（阻止进一步传递）
    bool Handled = false;
    
    // 事件源（原始触发控件�?
    Control* Source = nullptr;
    
    // 事件当前所在控�?
    Control* OriginalSource = nullptr;
};

// 鼠标事件参数
class MouseEventArgs : public RoutedEventArgs {
public:
    MouseEventArgs(float x, float y, int button = 0, int clicks = 1)
        : Position{x, y}, Button(button), Clicks(clicks) {}

    struct Point {
        float X, Y;
        Point(float x = 0, float y = 0) : X(x), Y(y) {}
    };

    Point Position;
    int Button = 0;      // 0=左键, 1=右键, 2=中键
    int Clicks = 1;      // 点击次数（用于双击检测）
    bool LeftButtonDown = false;
    bool RightButtonDown = false;
    bool MiddleButtonDown = false;
};

// 键盘事件参数
class KeyEventArgs : public RoutedEventArgs {
public:
    KeyEventArgs(int keyCode, bool isRepeat = false)
        : KeyCode(keyCode), IsRepeat(isRepeat) {}

    int KeyCode = 0;           // 虚拟键码
    bool IsRepeat = false;         // 是否重复按键
    bool IsSystemKey = false;  // 是否是系统键（Alt+Key�?
    
    // 修饰键状�?
    bool Control = false;
    bool Shift = false;
    bool Alt = false;
};

// 焦点事件参数
class FocusEventArgs : public RoutedEventArgs {
public:
    FocusEventArgs(Control* oldFocus, Control* newFocus)
        : OldFocus(oldFocus), NewFocus(newFocus) {}

    Control* OldFocus = nullptr;
    Control* NewFocus = nullptr;
};

// 事件路由策略
enum class RoutingStrategy {
    Direct,     // 直接发送到目标控件
    Bubble,     // 冒泡：目�?-> 父控�?-> �?
    Tunnel      // 隧道：根 -> 父控�?-> 目标（预览）
};

// 路由事件定义
class RoutedEvent {
public:
    using Id = uint32_t;
    
    RoutedEvent(const std::string& name, RoutingStrategy strategy)
        : m_name(name), m_strategy(strategy), m_id(s_nextId++) {}

    const std::string& GetName() const { return m_name; }
    RoutingStrategy GetStrategy() const { return m_strategy; }
    Id GetId() const { return m_id; }

private:
    std::string m_name;
    RoutingStrategy m_strategy;
    Id m_id;
    static Id s_nextId;
};

// 预定义路由事�?
namespace Events {
    // 鼠标事件
    extern const RoutedEvent PreviewMouseDown;  // 隧道
    extern const RoutedEvent MouseDown;          // 冒泡
    extern const RoutedEvent PreviewMouseUp;
    extern const RoutedEvent MouseUp;
    extern const RoutedEvent PreviewMouseMove;
    extern const RoutedEvent MouseMove;
    extern const RoutedEvent PreviewMouseWheel;
    extern const RoutedEvent MouseWheel;
    extern const RoutedEvent MouseEnter;         // 直接
    extern const RoutedEvent MouseLeave;         // 直接
    
    // 键盘事件
    extern const RoutedEvent PreviewKeyDown;
    extern const RoutedEvent KeyDown;
    extern const RoutedEvent PreviewKeyUp;
    extern const RoutedEvent KeyUp;
    extern const RoutedEvent PreviewTextInput;
    extern const RoutedEvent TextInput;
    
    // 焦点事件
    extern const RoutedEvent PreviewGotFocus;
    extern const RoutedEvent GotFocus;
    extern const RoutedEvent PreviewLostFocus;
    extern const RoutedEvent LostFocus;
    
    // 触摸/手势事件
    extern const RoutedEvent TouchDown;
    extern const RoutedEvent TouchUp;
    extern const RoutedEvent TouchMove;
}

// 事件处理器类�?
template<typename TArgs>
using EventHandler = std::function<void(Control* sender, TArgs& args)>;

// 事件路由条目
class EventRoute {
public:
    struct Entry {
        Control* Target;
        bool IsTunnel;  // true = 隧道阶段, false = 冒泡阶段
    };

    void Add(Control* control, bool isTunnel) {
        m_entries.push_back({control, isTunnel});
    }

    void Clear() { m_entries.clear(); }
    
    const std::vector<Entry>& GetEntries() const { return m_entries; }

private:
    std::vector<Entry> m_entries;
};

// 事件路由管理�?
class EventRouter {
public:
    // 构建路由路径
    static void BuildRoute(Control* target, EventRoute& route, RoutingStrategy strategy);
    
    // 引发事件
    static void RaiseEvent(Control* target, const RoutedEvent& routedEvent, RoutedEventArgs& args);
};

} // namespace controls
} // namespace luaui
