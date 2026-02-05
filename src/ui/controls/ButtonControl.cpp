/**
 * @file ButtonControl.cpp
 * @brief 按钮控件实现
 */

#include "ui/controls/ButtonControl.h"
#include <afxwin.h> // MFC support

namespace LuaUI {
namespace UI {

class ButtonControl::Impl {
public:
    CButton* button; ///< MFC按钮控件指针
    std::string text; ///< 按钮文本

    Impl() : button(nullptr), text("Button") { }
    ~Impl() {
        if (button) {
            button->DestroyWindow();
            button = nullptr;
        }
    }
};

ButtonControl::ButtonControl()
    : m_impl(new Impl())
{
}

ButtonControl::~ButtonControl() {
    delete m_impl;
}

std::string ButtonControl::getType() const {
    return "button";
}

bool ButtonControl::createFromXml(Xml::XmlElement* xmlElement, CWnd* parent) {
    // 调用基类实现
    bool result = BaseControl::createFromXml(xmlElement, parent);

    if (xmlElement) {
        // 获取按钮文本
        std::string text = xmlElement->getAttribute("text");
        if (!text.empty()) {
            setText(text);
        }

        // 创建MFC按钮
        createButton(parent);
    }

    return result;
}

void ButtonControl::bindLuaFunctions(lua_State* /*lua*/) {
    // 按钮控件的Lua函数绑定
}

CWnd* ButtonControl::getWindow() {
    return m_impl->button;
}

const CWnd* ButtonControl::getWindow() const {
    return m_impl->button;
}

void ButtonControl::setText(const std::string& text) {
    m_impl->text = text;
    if (m_impl->button) {
        m_impl->button->SetWindowTextA(text.c_str());
    }
}

std::string ButtonControl::getText() const {
    return m_impl->text;
}

BaseControl* ButtonControl::createInstance() {
    return new ButtonControl();
}

bool ButtonControl::createButton(CWnd* parent) {
    if (!parent || m_impl->button) {
        return false; // 父窗口不存在或按钮已存在
    }

    // 创建MFC按钮控件
    m_impl->button = new CButton();
    
    // 创建按钮
    CString text = CString(m_impl->text.c_str());
    if (!m_impl->button->Create(text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                               CRect(m_x, m_y, m_x + m_width, m_y + m_height),
                               parent, AFX_IDW_PANE_FIRST + 2)) {  // 使用唯一ID
        delete m_impl->button;
        m_impl->button = nullptr;
        return false;
    }

    // 显示按钮
    m_impl->button->ShowWindow(SW_SHOW);
    m_impl->button->UpdateWindow();

    return true;
}

} // namespace UI
} // namespace LuaUI