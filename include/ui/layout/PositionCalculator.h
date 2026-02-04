/**
 * @file PositionCalculator.h
 * @brief 位置计算器
 * @details 提供各种位置和尺寸计算工具函数
 * @version 1.0.0
 */

#ifndef LUAUI_POSITIONCALCULATOR_H
#define LUAUI_POSITIONCALCULATOR_H

#include "ui/layout/LayoutEngine.h"
#include "ui/controls/BaseControl.h"

namespace LuaUI {
namespace Layout {

/**
 * @brief 位置计算器类
 */
class PositionCalculator {
public:
    /**
     * @brief 计算控件的绝对位置
     * @param control 控件
     * @return 绝对位置
     */
    static Point calculateAbsolutePosition(UI::BaseControl* control);
    
    /**
     * @brief 计算控件的首选大小
     * @param control 控件
     * @return 首选大小
     */
    static Size calculatePreferredSize(UI::BaseControl* control);
    
    /**
     * @brief 计算控件的最小大小
     * @param control 控件
     * @return 最小大小
     */
    static Size calculateMinimumSize(UI::BaseControl* control);
    
    /**
     * @brief 计算控件的最大大小
     * @param control 控件
     * @return 最大大小
     */
    static Size calculateMaximumSize(UI::BaseControl* control);
    
    /**
     * @brief 调整矩形以适应边距
     * @param rect 原始矩形
     * @param margins 边距
     * @return 调整后的矩形
     */
    static Rect applyMargins(const Rect& rect, const Margins& margins);
    
    /**
     * @brief 调整矩形以适应内边距
     * @param rect 原始矩形
     * @param padding 内边距
     * @return 调整后的矩形
     */
    static Rect applyPadding(const Rect& rect, const Margins& padding);
    
    /**
     * @brief 居中矩形
     * @param rect 要居中的矩形
     * @param containerRect 容器矩形
     * @return 居中后的矩形
     */
    static Rect centerRect(const Rect& rect, const Rect& containerRect);
    
    /**
     * @brief 对齐矩形
     * @param rect 要对齐的矩形
     * @param containerRect 容器矩形
     * @param horizontal 水平对齐方式（-1左，0中，1右）
     * @param vertical 垂直对齐方式（-1上，0中，1下）
     * @return 对齐后的矩形
     */
    static Rect alignRect(const Rect& rect, const Rect& containerRect,
                         int horizontal, int vertical);
    
    /**
     * @brief 计算网格布局位置
     * @param index 索引
     * @param columns 列数
     * @param cellWidth 单元格宽度
     * @param cellHeight 单元格高度
     * @param spacing 间距
     * @return 位置
     */
    static Point calculateGridPosition(int index, int columns,
                                      int cellWidth, int cellHeight,
                                      int spacing);
    
    /**
     * @brief 计算垂直布局位置
     * @param index 索引
     * @param itemHeight 项目高度
     * @param spacing 间距
     * @return 位置
     */
    static Point calculateVerticalPosition(int index, int itemHeight, int spacing);
    
    /**
     * @brief 计算水平布局位置
     * @param index 索引
     * @param itemWidth 项目宽度
     * @param spacing 间距
     * @return 位置
     */
    static Point calculateHorizontalPosition(int index, int itemWidth, int spacing);
    
    /**
     * @brief 限制值在范围内
     * @param value 值
     * @param min 最小值
     * @param max 最大值
     * @return 限制后的值
     */
    static int clamp(int value, int min, int max);
};

} // namespace Layout
} // namespace LuaUI

#endif // LUAUI_POSITIONCALCULATOR_H
