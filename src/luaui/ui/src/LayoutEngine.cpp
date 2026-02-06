/**
 * @file LayoutEngine.cpp
 * @brief 布局引擎实现
 */

#include "ui/layout/LayoutEngine.h"
#include "ui/layout/ContainerLayout.h"
#include "ui/layout/PositionCalculator.h"
#include "ui/factory/ControlFactory.h"
#include "ui/controls/WindowControl.h"
#include "ui/controls/ButtonControl.h"
#include "ui/controls/LabelControl.h"
#include "core/ScriptEngine.h"
#include "core/LuaState.h"
#include "ui/controls/BaseControl.h"
#include "utils/Logger.h"
#include "lua/binding/ControlBinder.h"
#include <iostream>
#include <afxwin.h> // MFC support for ShowWindow and SW_SHOW

// 前向声明
namespace LuaUI {
namespace UI {
class WindowControl;
}
}

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

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

void LayoutEngine::showUI() {
    if (!m_rootControl) {
        LOG_ERROR("showUI - No root control!");
        return;
    }
    
    LOG_S_INFO_CAT("LayoutEngine") << "showUI: Starting... Root control type: " << m_rootControl->getType();
    
    // 打印所有控件信息用于调试
    std::vector<UI::BaseControl*> allControls = getAllControls();
    LOG_S_DEBUG_CAT("LayoutEngine") << "Total controls in layout: " << allControls.size();
    for (size_t i = 0; i < allControls.size(); ++i) {
        LOG_S_DEBUG_CAT("LayoutEngine") << "  Control " << i << ": " << allControls[i]->getType() 
                  << " (id: " << allControls[i]->getId() << ", pos: "
                  << allControls[i]->getX() << "," << allControls[i]->getY() 
                  << ", size: " << allControls[i]->getWidth() << "x" << allControls[i]->getHeight() << ")";
    }
    
    // 如果是窗口，确保窗口已创建并且子控件也已创建
    if (m_rootControl->getType() == "window") {
        UI::WindowControl* windowControl = dynamic_cast<UI::WindowControl*>(m_rootControl);
        if (!windowControl) {
            LOG_ERROR("showUI - Root control is not a WindowControl!");
            return;
        }
        
        LOG_S_INFO_CAT("LayoutEngine") << "Found WindowControl, title: " << windowControl->getTitle();
        
        // 如果窗口还没有创建，先创建窗口
        if (!windowControl->getWindow()) {
            LOG_INFO_CAT("LayoutEngine", "Creating main window...");
            if (!windowControl->createWindow(nullptr)) {
                LOG_ERROR("Failed to create main window!");
                return;
            }
            LOG_INFO_CAT("LayoutEngine", "Main window created successfully");
        } else {
            LOG_INFO_CAT("LayoutEngine", "Main window already exists");
        }
        
        CWnd* window = windowControl->getWindow();
        if (!window || !::IsWindow(window->m_hWnd)) {
            LOG_ERROR("Window handle is invalid!");
            return;
        }
        
        LOG_S_DEBUG_CAT("LayoutEngine") << "Window handle: " << window->m_hWnd;
        
        // 创建所有子控件的MFC窗口（无论窗口是否已存在）
        LOG_INFO_CAT("LayoutEngine", "Creating child windows...");
        windowControl->createChildWindows();
        
        // 显示主窗口
        LOG_INFO_CAT("LayoutEngine", "Showing window...");
        window->ShowWindow(SW_SHOW);
        window->UpdateWindow();
        
        // 强制处理消息以确保窗口立即显示
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        LOG_INFO_CAT("LayoutEngine", "showUI completed successfully");
    } else {
        LOG_S_INFO_CAT("LayoutEngine") << "Root control is not a window, type: " << m_rootControl->getType();
        // 非窗口根控件，直接显示所有控件
        showAllControls(m_rootControl);
    }
}

void LayoutEngine::showAllControls(UI::BaseControl* control) {
    if (!control) {
        return;
    }
    
    // 显示当前控件
    CWnd* wnd = control->getWindow();
    if (wnd && ::IsWindow(wnd->m_hWnd)) {
        LOG_S_DEBUG_CAT("LayoutEngine") << "Showing control: " << control->getId();
        wnd->ShowWindow(SW_SHOW);
        wnd->UpdateWindow();
    }
    
    // 递归显示子控件
    UI::WindowControl* window = dynamic_cast<UI::WindowControl*>(control);
    if (window) {
        std::vector<UI::BaseControl*> children = getAllControls();
        for (size_t i = 0; i < children.size(); ++i) {
            if (children[i] != control) { // 避免重复显示
                showAllControls(children[i]);
            }
        }
    }
}

UI::BaseControl* LayoutEngine::createControlTree(Xml::XmlElement* xmlElement, UI::BaseControl* parent) {
    if (!xmlElement) {
        LOG_S_WARN_CAT("LayoutEngine") << "createControlTree: xmlElement is null";
        return nullptr;
    }
    
    std::string elementType = xmlElement->getType();
    std::string elementId = xmlElement->getAttribute("id");
    LOG_S_DEBUG_CAT("LayoutEngine") << "Creating control: type=" << elementType << ", id=" << elementId;
    
    // 使用工厂创建控件
    UI::BaseControl* control = UI::ControlFactory::instance().createFromXml(xmlElement, nullptr);
    
    if (!control) {
        LOG_S_ERROR_CAT("LayoutEngine") << "Failed to create control: type=" << elementType;
        return nullptr;
    }
    
    // 记录控件
    std::string id = control->getId();
    LOG_S_DEBUG_CAT("LayoutEngine") << "Control created: type=" << control->getType() << ", id=" << id;
    
    if (!id.empty()) {
        m_controls[id] = control;
        // 注册控件到 Lua
        Lua::Binding::ControlBinder::registerControl(control, id);
        LOG_S_INFO_CAT("LayoutEngine") << "Registered control: " << id << " (type: " << control->getType() << ")";
    } else {
        LOG_S_WARN_CAT("LayoutEngine") << "Control ID is empty, skipping registration (type: " << control->getType() << ")";
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

void LayoutEngine::bindLuaEvents(IScriptEngine* scriptEngine) {
    // 空实现 - 事件绑定在main.cpp中直接调用
    (void)scriptEngine;
}

} // namespace Layout
} // namespace LuaUI
