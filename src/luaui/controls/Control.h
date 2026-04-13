#pragma once

// Compatibility header - forwards to core Control.h
#include "../core/Control.h"

namespace luaui {
namespace controls {

// Event arguments structures for input handling
struct KeyEventArgs {
    int keyCode = 0;
    bool isRepeat = false;
    bool Control = false;
    bool Shift = false;
    bool Alt = false;
    bool Handled = false;
};

struct MouseEventArgs {
    float x = 0, y = 0;
    int button = 0;
    bool Handled = false;
};

// 动画时长常量（毫秒）
constexpr float kHoverAnimMs  = 40.0f;   // 鼠标划出
constexpr float kPressAnimMs  = 50.0f;   // 按下/释放

} // namespace controls
} // namespace luaui
