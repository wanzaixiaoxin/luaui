#include "Event.h"
#include "Control.h"

namespace luaui {
namespace controls {

// RoutedEvent ID 生成�?
RoutedEvent::Id RoutedEvent::s_nextId = 1;

// ==================== 预定义路由事�?====================
namespace Events {
    // 鼠标事件
    const RoutedEvent PreviewMouseDown("PreviewMouseDown", RoutingStrategy::Tunnel);
    const RoutedEvent MouseDown("MouseDown", RoutingStrategy::Bubble);
    const RoutedEvent PreviewMouseUp("PreviewMouseUp", RoutingStrategy::Tunnel);
    const RoutedEvent MouseUp("MouseUp", RoutingStrategy::Bubble);
    const RoutedEvent PreviewMouseMove("PreviewMouseMove", RoutingStrategy::Tunnel);
    const RoutedEvent MouseMove("MouseMove", RoutingStrategy::Bubble);
    const RoutedEvent PreviewMouseWheel("PreviewMouseWheel", RoutingStrategy::Tunnel);
    const RoutedEvent MouseWheel("MouseWheel", RoutingStrategy::Bubble);
    const RoutedEvent MouseEnter("MouseEnter", RoutingStrategy::Direct);
    const RoutedEvent MouseLeave("MouseLeave", RoutingStrategy::Direct);
    
    // 键盘事件
    const RoutedEvent PreviewKeyDown("PreviewKeyDown", RoutingStrategy::Tunnel);
    const RoutedEvent KeyDown("KeyDown", RoutingStrategy::Bubble);
    const RoutedEvent PreviewKeyUp("PreviewKeyUp", RoutingStrategy::Tunnel);
    const RoutedEvent KeyUp("KeyUp", RoutingStrategy::Bubble);
    const RoutedEvent PreviewTextInput("PreviewTextInput", RoutingStrategy::Tunnel);
    const RoutedEvent TextInput("TextInput", RoutingStrategy::Bubble);
    
    // 焦点事件
    const RoutedEvent PreviewGotFocus("PreviewGotFocus", RoutingStrategy::Tunnel);
    const RoutedEvent GotFocus("GotFocus", RoutingStrategy::Bubble);
    const RoutedEvent PreviewLostFocus("PreviewLostFocus", RoutingStrategy::Tunnel);
    const RoutedEvent LostFocus("LostFocus", RoutingStrategy::Bubble);
    
    // 触摸事件
    const RoutedEvent TouchDown("TouchDown", RoutingStrategy::Bubble);
    const RoutedEvent TouchUp("TouchUp", RoutingStrategy::Bubble);
    const RoutedEvent TouchMove("TouchMove", RoutingStrategy::Bubble);
}

// ==================== EventRouter ====================
void EventRouter::BuildRoute(Control* target, EventRoute& route, RoutingStrategy strategy) {
    route.Clear();
    
    if (!target) return;
    
    // 收集从根到目标的路径
    std::vector<Control*> path;
    Control* current = target;
    while (current) {
        path.push_back(current);
        auto parent = current->GetParent();
        current = parent.get();
    }
    
    // 根据策略构建路由
    switch (strategy) {
        case RoutingStrategy::Direct:
            // 直接事件：只发送到目标
            route.Add(target, false);
            break;
            
        case RoutingStrategy::Tunnel:
            // 隧道事件：从根到目标
            for (auto it = path.rbegin(); it != path.rend(); ++it) {
                route.Add(*it, true);
            }
            break;
            
        case RoutingStrategy::Bubble:
            // 冒泡事件：从目标到根
            for (auto it = path.begin(); it != path.end(); ++it) {
                route.Add(*it, false);
            }
            break;
    }
}

void EventRouter::RaiseEvent(Control* target, const RoutedEvent& routedEvent, RoutedEventArgs& args) {
    if (!target) return;
    
    // 设置事件�?
    args.Source = target;
    args.Handled = false;
    
    // 构建路由
    EventRoute route;
    BuildRoute(target, route, routedEvent.GetStrategy());
    
    // 调用路由中的每个控件
    for (const auto& entry : route.GetEntries()) {
        if (args.Handled) break;
        
        args.OriginalSource = entry.Target;
        entry.Target->RaiseEvent(routedEvent, args);
    }
}

} // namespace controls
} // namespace luaui
