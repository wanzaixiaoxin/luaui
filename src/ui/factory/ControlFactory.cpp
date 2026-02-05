/**
 * @file ControlFactory.cpp
 * @brief 控件工厂实现
 */

#include "ui/factory/ControlFactory.h"
#include "ui/controls/GenericControl.h"
#include "ui/controls/WindowControl.h"
#include "ui/controls/ButtonControl.h"
#include "ui/controls/LabelControl.h"

namespace LuaUI {
namespace UI {

ControlFactory::ControlFactory() {
    registerBuiltinControls();
}

ControlFactory::~ControlFactory() {
    m_creators.clear();
}

ControlFactory& ControlFactory::instance() {
    static ControlFactory factory;
    return factory;
}

BaseControl* ControlFactory::createControl(const std::string& type) {
    std::map<std::string, ControlCreator>::iterator it = m_creators.find(type);
    if (it != m_creators.end()) {
        return it->second();
    }
    return nullptr;
}

BaseControl* ControlFactory::createFromXml(Xml::XmlElement* xmlElement, CWnd* parent) {
    if (!xmlElement) {
        return nullptr;
    }
    
    std::string type = xmlElement->getType();
    BaseControl* control = createControl(type);
    
    if (control) {
        control->createFromXml(xmlElement, parent);
    }
    
    return control;
}

void ControlFactory::registerControl(const std::string& type, ControlCreator creator) {
    if (creator) {
        m_creators[type] = creator;
    }
}

bool ControlFactory::isRegistered(const std::string& type) const {
    return m_creators.find(type) != m_creators.end();
}

std::vector<std::string> ControlFactory::getRegisteredTypes() const {
    std::vector<std::string> types;
    for (std::map<std::string, ControlCreator>::const_iterator it = m_creators.begin();
         it != m_creators.end(); ++it) {
        types.push_back(it->first);
    }
    return types;
}

bool ControlFactory::unregisterControl(const std::string& type) {
    std::map<std::string, ControlCreator>::iterator it = m_creators.find(type);
    if (it != m_creators.end()) {
        m_creators.erase(it);
        return true;
    }
    return false;
}

void ControlFactory::registerBuiltinControls() {
    // 注册内置控件类型到具体实现
    registerControl("window", &WindowControl::createInstance);
    registerControl("button", &ButtonControl::createInstance);
    registerControl("label", &LabelControl::createInstance);
    registerControl("edit", &GenericControl::createInstance);  // 编辑框暂时使用通用控件
    registerControl("menu", &GenericControl::createInstance);  // 菜单暂时使用通用控件
    registerControl("menuitem", &GenericControl::createInstance);  // 菜单项暂时使用通用控件
    registerControl("toolbar", &GenericControl::createInstance);  // 工具栏暂时使用通用控件
    registerControl("toolbutton", &GenericControl::createInstance);  // 工具按钮暂时使用通用控件
    registerControl("statusbar", &GenericControl::createInstance);  // 状态栏暂时使用通用控件
    registerControl("pane", &GenericControl::createInstance);  // 面板暂时使用通用控件
}

} // namespace UI
} // namespace LuaUI
