/**
 * @file ContainerLayout.cpp
 * @brief 容器布局实现
 */

#include "ui/layout/ContainerLayout.h"
#include "ui/layout/PositionCalculator.h"
#include <algorithm>

namespace LuaUI {
namespace Layout {

ContainerLayout::ContainerLayout(UI::BaseControl* container)
    : m_container(container)
    , m_layoutType(LAYOUT_ABSOLUTE)
    , m_spacing(5)
{
}

ContainerLayout::~ContainerLayout() {
    m_children.clear();
}

void ContainerLayout::addChild(UI::BaseControl* child) {
    if (child) {
        m_children.push_back(child);
    }
}

void ContainerLayout::removeChild(UI::BaseControl* child) {
    std::vector<UI::BaseControl*>::iterator it = std::find(m_children.begin(), 
                                                           m_children.end(), 
                                                           child);
    if (it != m_children.end()) {
        m_children.erase(it);
    }
}

Rect ContainerLayout::layout(const Rect& availableRect) {
    // 应用边距
    Rect contentRect = PositionCalculator::applyMargins(availableRect, m_margins);
    
    // 根据布局类型计算
    switch (m_layoutType) {
        case LAYOUT_VERTICAL:
            return layoutVertical(contentRect);
        case LAYOUT_HORIZONTAL:
            return layoutHorizontal(contentRect);
        case LAYOUT_ABSOLUTE:
        default:
            return layoutAbsolute(contentRect);
    }
}

void ContainerLayout::setLayoutType(LayoutType type) {
    m_layoutType = type;
}

LayoutType ContainerLayout::getLayoutType() const {
    return m_layoutType;
}

void ContainerLayout::setSpacing(int spacing) {
    m_spacing = spacing;
}

int ContainerLayout::getSpacing() const {
    return m_spacing;
}

void ContainerLayout::setMargins(const Margins& margins) {
    m_margins = margins;
}

const Margins& ContainerLayout::getMargins() const {
    return m_margins;
}

Rect ContainerLayout::layoutVertical(const Rect& availableRect) {
    int currentY = availableRect.y;
    
    // 遍历所有子控件
    for (size_t i = 0; i < m_children.size(); ++i) {
        UI::BaseControl* child = m_children[i];
        
        if (!child) {
            continue;
        }
        
        // 获取子控件的首选高度
        Size preferredSize = PositionCalculator::calculatePreferredSize(child);
        
        // 计算子控件的位置和大小
        int x = availableRect.x;
        int y = currentY;
        int width = availableRect.width;
        int height = (preferredSize.height > 0) ? preferredSize.height : 30;
        
        // 设置子控件位置和大小
        child->setPosition(x, y);
        child->setSize(width, height);
        
        // 更新Y坐标
        currentY += height + m_spacing;
    }
    
    // 返回布局后的总矩形
    Rect layoutRect = availableRect;
    layoutRect.height = currentY - availableRect.y - m_spacing;
    
    return layoutRect;
}

Rect ContainerLayout::layoutHorizontal(const Rect& availableRect) {
    int currentX = availableRect.x;
    
    // 遍历所有子控件
    for (size_t i = 0; i < m_children.size(); ++i) {
        UI::BaseControl* child = m_children[i];
        
        if (!child) {
            continue;
        }
        
        // 获取子控件的首选宽度
        Size preferredSize = PositionCalculator::calculatePreferredSize(child);
        
        // 计算子控件的位置和大小
        int x = currentX;
        int y = availableRect.y;
        int width = (preferredSize.width > 0) ? preferredSize.width : 100;
        int height = availableRect.height;
        
        // 设置子控件位置和大小
        child->setPosition(x, y);
        child->setSize(width, height);
        
        // 更新X坐标
        currentX += width + m_spacing;
    }
    
    // 返回布局后的总矩形
    Rect layoutRect = availableRect;
    layoutRect.width = currentX - availableRect.x - m_spacing;
    
    return layoutRect;
}

Rect ContainerLayout::layoutAbsolute(const Rect& availableRect) {
    // 绝对布局：使用控件自身的x、y属性
    for (size_t i = 0; i < m_children.size(); ++i) {
        UI::BaseControl* child = m_children[i];
        
        if (!child) {
            continue;
        }
        
        // 获取控件的位置和大小
        int x = atoi(child->getProperty("x").c_str());
        int y = atoi(child->getProperty("y").c_str());
        int width = atoi(child->getProperty("width").c_str());
        int height = atoi(child->getProperty("height").c_str());
        
        // 设置控件位置和大小
        child->setPosition(x, y);
        child->setSize(width, height);
    }
    
    return availableRect;
}

} // namespace Layout
} // namespace LuaUI
