/**
 * @file ContainerLayout.h
 * @brief 容器布局类
 * @details 处理容器控件的子控件布局
 * @version 1.0.0
 */

#ifndef LUAUI_CONTAINERLAYOUT_H
#define LUAUI_CONTAINERLAYOUT_H

#include "ui/layout/LayoutEngine.h"
#include "ui/controls/BaseControl.h"

namespace LuaUI {
namespace Layout {

/**
 * @brief 容器布局类
 */
class ContainerLayout {
public:
    /**
     * @brief 构造函数
     * @param container 容器控件
     */
    ContainerLayout(UI::BaseControl* container);
    
    /**
     * @brief 析构函数
     */
    ~ContainerLayout();
    
    /**
     * @brief 添加子控件
     * @param child 子控件
     */
    void addChild(UI::BaseControl* child);
    
    /**
     * @brief 移除子控件
     * @param child 子控件
     */
    void removeChild(UI::BaseControl* child);
    
    /**
     * @brief 计算布局
     * @param availableRect 可用区域
     * @return 布局后的矩形
     */
    Rect layout(const Rect& availableRect);
    
    /**
     * @brief 设置布局类型
     * @param type 布局类型
     */
    void setLayoutType(LayoutType type);
    
    /**
     * @brief 获取布局类型
     * @return 布局类型
     */
    LayoutType getLayoutType() const;
    
    /**
     * @brief 设置间距
     * @param spacing 间距值
     */
    void setSpacing(int spacing);
    
    /**
     * @brief 获取间距
     * @return 间距值
     */
    int getSpacing() const;
    
    /**
     * @brief 设置边距
     * @param margins 边距
     */
    void setMargins(const Margins& margins);
    
    /**
     * @brief 获取边距
     * @return 边距
     */
    const Margins& getMargins() const;
    
private:
    UI::BaseControl* m_container;                   ///< 容器控件
    std::vector<UI::BaseControl*> m_children;      ///< 子控件列表
    LayoutType m_layoutType;                        ///< 布局类型
    int m_spacing;                                 ///< 间距
    Margins m_margins;                             ///< 边距
    
    /**
     * @brief 计算垂直布局
     * @param availableRect 可用区域
     * @return 布局后的矩形
     */
    Rect layoutVertical(const Rect& availableRect);
    
    /**
     * @brief 计算水平布局
     * @param availableRect 可用区域
     * @return 布局后的矩形
     */
    Rect layoutHorizontal(const Rect& availableRect);
    
    /**
     * @brief 计算绝对布局
     * @param availableRect 可用区域
     * @return 布局后的矩形
     */
    Rect layoutAbsolute(const Rect& availableRect);
};

} // namespace Layout
} // namespace LuaUI

#endif // LUAUI_CONTAINERLAYOUT_H
