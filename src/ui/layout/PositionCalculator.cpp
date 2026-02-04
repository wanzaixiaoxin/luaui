/**
 * @file PositionCalculator.cpp
 * @brief 位置计算器实现
 */

#include "ui/layout/PositionCalculator.h"
#include <algorithm>

namespace LuaUI {
namespace Layout {

Point PositionCalculator::calculateAbsolutePosition(UI::BaseControl* control) {
    if (!control) {
        return Point();
    }
    
    // 获取控件的相对位置
    int x = atoi(control->getProperty("x").c_str());
    int y = atoi(control->getProperty("y").c_str());
    
    // TODO: 递归累加父控件的位置
    // 需要父控件的引用
    
    return Point(x, y);
}

Size PositionCalculator::calculatePreferredSize(UI::BaseControl* control) {
    if (!control) {
        return Size();
    }
    
    // 从XML属性获取大小
    int width = atoi(control->getProperty("width").c_str());
    int height = atoi(control->getProperty("height").c_str());
    
    return Size(width, height);
}

Size PositionCalculator::calculateMinimumSize(UI::BaseControl* control) {
    if (!control) {
        return Size();
    }
    
    // 返回默认最小尺寸
    return Size(50, 20);
}

Size PositionCalculator::calculateMaximumSize(UI::BaseControl* control) {
    if (!control) {
        return Size();
    }
    
    // 返回默认最大尺寸
    return Size(32767, 32767);
}

Rect PositionCalculator::applyMargins(const Rect& rect, const Margins& margins) {
    Rect result = rect;
    
    result.x += margins.left;
    result.y += margins.top;
    result.width -= (margins.left + margins.right);
    result.height -= (margins.top + margins.bottom);
    
    return result;
}

Rect PositionCalculator::applyPadding(const Rect& rect, const Margins& padding) {
    return applyMargins(rect, padding);
}

Rect PositionCalculator::centerRect(const Rect& rect, const Rect& containerRect) {
    Rect result = rect;
    
    // 计算居中位置
    result.x = containerRect.x + (containerRect.width - rect.width) / 2;
    result.y = containerRect.y + (containerRect.height - rect.height) / 2;
    
    return result;
}

Rect PositionCalculator::alignRect(const Rect& rect, const Rect& containerRect,
                                  int horizontal, int vertical) {
    Rect result = rect;
    
    // 水平对齐
    if (horizontal == -1) {
        // 左对齐
        result.x = containerRect.x;
    } else if (horizontal == 1) {
        // 右对齐
        result.x = containerRect.x + containerRect.width - rect.width;
    } else {
        // 居中对齐
        result.x = containerRect.x + (containerRect.width - rect.width) / 2;
    }
    
    // 垂直对齐
    if (vertical == -1) {
        // 顶对齐
        result.y = containerRect.y;
    } else if (vertical == 1) {
        // 底对齐
        result.y = containerRect.y + containerRect.height - rect.height;
    } else {
        // 居中对齐
        result.y = containerRect.y + (containerRect.height - rect.height) / 2;
    }
    
    return result;
}

Point PositionCalculator::calculateGridPosition(int index, int columns,
                                            int cellWidth, int cellHeight,
                                            int spacing) {
    Point position;
    
    int row = index / columns;
    int col = index % columns;
    
    position.x = col * (cellWidth + spacing);
    position.y = row * (cellHeight + spacing);
    
    return position;
}

Point PositionCalculator::calculateVerticalPosition(int index, int itemHeight, int spacing) {
    Point position;
    
    position.x = 0;
    position.y = index * (itemHeight + spacing);
    
    return position;
}

Point PositionCalculator::calculateHorizontalPosition(int index, int itemWidth, int spacing) {
    Point position;
    
    position.x = index * (itemWidth + spacing);
    position.y = 0;
    
    return position;
}

int PositionCalculator::clamp(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

} // namespace Layout
} // namespace LuaUI
