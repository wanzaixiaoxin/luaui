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

} // namespace controls
} // namespace luaui
