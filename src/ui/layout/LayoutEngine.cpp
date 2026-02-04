/**
 * @file LayoutEngine.cpp
 * @brief 布局引擎实现
 */

#include "ui/layout/LayoutEngine.h"
#include "ui/layout/ContainerLayout.h"
#include "ui/layout/PositionCalculator.h"
#include "ui/factory/ControlFactory.h"
#include <algorithm>

namespace LuaUI {
namespace Layout {

LayoutEngine::LayoutEngine()
    : m_rootControl(nullptr)
{
}

LayoutEngine::~LayoutEngine() {
    // 清理控件
    for (std::map<std::string, UI::BaseControl*>::iterator it = m_controls.begin();
         it != m_controls.end(); ++it) {
        delete it->second;
    }
    m_controls.clear();
}

bool LayoutEngine::createLayout(Xml::XmlElement* xmlElement) {
    if (!xmlElement) {
        return false;
    }
    
    // 创建控件树
    m_rootControl = createControlTree(xmlElement, nullptr);
    
    if (!m_rootControl) {
        return false;
    }
    
    // 计算初始布局
    return calculateLayout();
}

bool LayoutEngine::calculateLayout() {
    if (!m_rootControl) {
        return false;
    }
    
    // 获取根控件的大小
    int width = atoi(m_rootControl->getProperty("width").c_str());
    int height = atoi(m_rootControl->getProperty("height").c_str());
    
    if (width == 0) width = 800;
    if (height == 0) height = 600;
    
    Rect rect(0, 0, width, height);
    
    // 递归计算布局
    calculateControlLayout(m_rootControl, rect);
    
    return true;
}

UI::BaseControl* LayoutEngine::getRootControl() {
    return m_rootControl;
}

UI::BaseControl* LayoutEngine::getControlById(const std::string& id) {
    std::map<std::string, UI::BaseControl*>::iterator it = m_controls.find(id);
    if (it != m_controls.end()) {
        return it->second;
    }
    
    // 递归查找
    if (m_rootControl) {
        return m_rootControl->findChildById(id);
    }
    
    return nullptr;
}

std::vector<UI::BaseControl*> LayoutEngine::getAllControls() {
    std::vector<UI::BaseControl*> controls;
    
    for (std::map<std::string, UI::BaseControl*>::iterator it = m_controls.begin();
         it != m_controls.end(); ++it) {
        controls.push_back(it->second);
    }
    
    return controls;
}

bool LayoutEngine::updateLayout() {
    return calculateLayout();
}

bool LayoutEngine::setLayoutType(const std::string& containerId, LayoutType layoutType) {
    m_layoutTypes[containerId] = layoutType;
    return true;
}

LayoutType LayoutEngine::getLayoutType(const std::string& containerId) {
    std::map<std::string, LayoutType>::iterator it = m_layoutTypes.find(containerId);
    if (it != m_layoutTypes.end()) {
        return it->second;
    }
    return LAYOUT_ABSOLUTE;
}

UI::BaseControl* LayoutEngine::createControlTree(Xml::XmlElement* xmlElement, UI::BaseControl* /*parent*/) {
    if (!xmlElement) {
        return nullptr;
    }
    
    // 使用工厂创建控件
    UI::BaseControl* control = UI::ControlFactory::instance().createFromXml(xmlElement);
    
    if (!control) {
        return nullptr;
    }
    
    // 记录控件
    std::string id = control->getId();
    if (!id.empty()) {
        m_controls[id] = control;
    }
    
    // 递归创建子控件
    const std::vector<Xml::XmlElement*>& children = xmlElement->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        UI::BaseControl* child = createControlTree(children[i], control);
        if (child) {
            control->addChild(child);
        }
    }
    
    return control;
}

void LayoutEngine::calculateControlLayout(UI::BaseControl* control, const Rect& rect) {
    if (!control) {
        return;
    }
    
    // 获取布局类型
    std::string layoutStr = control->getProperty("layout");
    LayoutType layoutType = parseLayoutType(layoutStr);
    
    // 根据布局类型计算
    switch (layoutType) {
        case LAYOUT_VERTICAL:
            calculateVerticalLayout(control, rect);
            break;
        case LAYOUT_HORIZONTAL:
            calculateHorizontalLayout(control, rect);
            break;
        case LAYOUT_ABSOLUTE:
        default:
            calculateAbsoluteLayout(control, rect);
            break;
    }
}

void LayoutEngine::calculateAbsoluteLayout(UI::BaseControl* control, const Rect& /*rect*/) {
    if (!control) {
        return;
    }
    
    // 使用控件的x、y属性
    int x = atoi(control->getProperty("x").c_str());
    int y = atoi(control->getProperty("y").c_str());
    int width = atoi(control->getProperty("width").c_str());
    int height = atoi(control->getProperty("height").c_str());
    
    // 设置控件位置和大小
    control->setPosition(x, y);
    control->setSize(width, height);
    
    // 递归处理子控件
    // 子控件的坐标相对于父控件
    // ...省略子控件处理
}

void LayoutEngine::calculateVerticalLayout(UI::BaseControl* control, const Rect& /*rect*/) {
    if (!control) {
        return;
    }
    
    // 垂直布局：子控件从上到下排列
    // int currentY = rect.y;
    int spacing = 5; // 默认间距
    
    std::string spacingStr = control->getProperty("spacing");
    if (!spacingStr.empty()) {
        spacing = atoi(spacingStr.c_str());
    }
    
    // 处理子控件
    // ...省略子控件处理
}

void LayoutEngine::calculateHorizontalLayout(UI::BaseControl* control, const Rect& /*rect*/) {
    if (!control) {
        return;
    }
    
    // 水平布局：子控件从左到右排列
    // int currentX = rect.x;
    int spacing = 5; // 默认间距
    
    std::string spacingStr = control->getProperty("spacing");
    if (!spacingStr.empty()) {
        spacing = atoi(spacingStr.c_str());
    }
    
    // 处理子控件
    // ...省略子控件处理
}

LayoutType LayoutEngine::parseLayoutType(const std::string& layoutStr) {
    if (layoutStr == "vertical" || layoutStr == "vbox") {
        return LAYOUT_VERTICAL;
    } else if (layoutStr == "horizontal" || layoutStr == "hbox") {
        return LAYOUT_HORIZONTAL;
    } else if (layoutStr == "grid") {
        return LAYOUT_GRID;
    } else if (layoutStr == "flex") {
        return LAYOUT_FLEX;
    }
    return LAYOUT_ABSOLUTE;
}

bool LayoutEngine::loadFromXml(const std::string& xmlFile) {
    // 使用XML解析器解析文件
    Xml::XmlParser parser;
    auto* rootElement = parser.parseFile(xmlFile.c_str());
    if (!rootElement) {
        return false;
    }
    
    // 创建布局
    return createLayout(rootElement);
}

bool LayoutEngine::loadFromXmlString(const std::string& xmlContent) {
    // 使用XML解析器解析字符串
    Xml::XmlParser parser;
    auto* rootElement = parser.parseString(xmlContent.c_str());
    if (!rootElement) {
        return false;
    }
    
    // 创建布局
    return createLayout(rootElement);
}

std::shared_ptr<IControl> LayoutEngine::getControl(const std::string& id) {
    // 使用内部方法获取控件，然后包装成共享指针
    UI::BaseControl* control = getControlById(id);
    if (control) {
        // 由于BaseControl现在继承自IControl，我们可以直接返回
        return std::shared_ptr<IControl>(dynamic_cast<IControl*>(control));
    }
    
    return nullptr;
}

} // namespace Layout
} // namespace LuaUI
