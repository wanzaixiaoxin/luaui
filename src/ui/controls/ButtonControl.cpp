/**
 * @file ButtonControl.cpp
 * @brief 按钮控件实现
 */

#include "ui/controls/ButtonControl.h"
#include "xml/parser/XmlParser.h"
#include <afxwin.h> // MFC support
#include <iostream>

namespace LuaUI {

namespace UI {

// LuaUIButton - MFC按钮类，用于处理消息
class LuaUIButton : public CButton {
    DECLARE_DYNCREATE(LuaUIButton)

public:
    LuaUIButton() : m_owner(nullptr) {}
    virtual ~LuaUIButton() {}

    void setOwnerControl(ButtonControl* owner) { m_owner = owner; }
    ButtonControl* getOwnerControl() const { return m_owner; }

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnClicked();

private:
    ButtonControl* m_owner;
};

// LuaUIButton 的消息映射实现
IMPLEMENT_DYNCREATE(LuaUIButton, CButton)

BEGIN_MESSAGE_MAP(LuaUIButton, CButton)
    ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
END_MESSAGE_MAP()

void LuaUIButton::OnClicked()
{
    if (m_owner) {
        std::cout << "Button clicked: " << m_owner->getId() << std::endl;
        // 触发按钮的点击事件
        m_owner->fireEvent("onClick");
    }
}

class ButtonControl::Impl {
public:
    LuaUIButton* button;
    std::string text;
    static int s_nextId;

    Impl() : button(nullptr), text("Button") { }
    
    static int getNextId() {
        return s_nextId++;
    }
    
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

        // 延迟创建 MFC 按钮，等到父窗口创建后再创建
        // createButton 会在 LayoutEngine::showUI 中被调用
        if (parent) {
            createButton(parent);
        }
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
    std::cout << "ButtonControl::createButton: parent=" << parent 
              << ", existing button=" << m_impl->button << std::endl;
    if (!parent || m_impl->button) {
        std::cout << "ButtonControl::createButton: Failed - "
                  << (!parent ? "no parent" : "button already exists") << std::endl;
        return false; // 父窗口不存在或按钮已存在
    }

    // 创建MFC按钮控件
    m_impl->button = new LuaUIButton();
    LuaUIButton* luaButton = static_cast<LuaUIButton*>(m_impl->button);
    luaButton->setOwnerControl(this);

    // 创建按钮，使用唯一 ID
    CString text = CString(m_impl->text.c_str());
    int controlId = m_impl->getNextId();
    std::cout << "ButtonControl::createButton: Creating with ID=" << controlId << std::endl;
    
    if (!luaButton->Create(text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                         CRect(m_x, m_y, m_x + m_width, m_y + m_height),
                         parent, controlId)) {
        delete m_impl->button;
        m_impl->button = nullptr;
        return false;
    }

    // 把按钮带到最前面
    m_impl->button->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    
    // 显示按钮
    m_impl->button->ShowWindow(SW_SHOW);
    m_impl->button->UpdateWindow();

    std::cout << "ButtonControl::createButton: Success!" << std::endl;
    return true;
}

// 初始化静态成员
int ButtonControl::Impl::s_nextId = 1000;

} // namespace UI
} // namespace LuaUI
