/**
 * @file LabelControl.cpp
 * @brief 标签控件实现
 */

#include "ui/controls/LabelControl.h"
#include <iostream>
#include <afxwin.h> // MFC support

namespace LuaUI {
namespace UI {

class LabelControl::Impl {
public:
    CStatic* label; ///< MFC标签控件指针
    std::string text; ///< 标签文本
    static int s_nextId; ///< 下一个控件ID

    Impl() : label(nullptr), text("Label") { }
    
    static int getNextId() {
        return s_nextId++;
    }
    ~Impl() {
        if (label) {
            label->DestroyWindow();
            label = nullptr;
        }
    }
};

LabelControl::LabelControl()
    : m_impl(new Impl())
{
}

LabelControl::~LabelControl() {
    delete m_impl;
}

std::string LabelControl::getType() const {
    return "label";
}

bool LabelControl::createFromXml(Xml::XmlElement* xmlElement, CWnd* parent) {
    // 调用基类实现
    bool result = BaseControl::createFromXml(xmlElement, parent);

    if (xmlElement) {
        // 获取标签文本
        std::string text = xmlElement->getAttribute("text");
        if (!text.empty()) {
            setText(text);
        }

        // 延迟创建 MFC 标签，等到父窗口创建后再创建
        // createLabel 会在 LayoutEngine::showUI 中被调用
        if (parent) {
            createLabel(parent);
        }
    }

    return result;
}

void LabelControl::bindLuaFunctions(lua_State* /*lua*/) {
    // 标签控件的Lua函数绑定
}

CWnd* LabelControl::getWindow() {
    return m_impl->label;
}

const CWnd* LabelControl::getWindow() const {
    return m_impl->label;
}

void LabelControl::setText(const std::string& text) {
    m_impl->text = text;
    if (m_impl->label) {
        m_impl->label->SetWindowTextA(text.c_str());
    }
}

std::string LabelControl::getText() const {
    return m_impl->text;
}

BaseControl* LabelControl::createInstance() {
    return new LabelControl();
}

bool LabelControl::createLabel(CWnd* parent) {
    std::cout << "LabelControl::createLabel: parent=" << parent 
              << ", existing label=" << m_impl->label << std::endl;
    if (!parent || m_impl->label) {
        std::cout << "LabelControl::createLabel: Failed - "
                  << (!parent ? "no parent" : "label already exists") << std::endl;
        return false; // 父窗口不存在或标签已存在
    }

    // 创建MFC标签控件
    m_impl->label = new CStatic();
    
    // 创建标签，使用唯一 ID
    CString text = CString(m_impl->text.c_str());
    int controlId = m_impl->getNextId();
    std::cout << "LabelControl::createLabel: Creating with ID=" << controlId << std::endl;
    if (!m_impl->label->Create(text, WS_CHILD | WS_VISIBLE | SS_LEFT,
                               CRect(m_x, m_y, m_x + m_width, m_y + m_height),
                               parent, controlId)) {
        delete m_impl->label;
        m_impl->label = nullptr;
        return false;
    }

    // 把标签带到最前面
    m_impl->label->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    
    // 显示标签
    m_impl->label->ShowWindow(SW_SHOW);
    m_impl->label->UpdateWindow();

    std::cout << "LabelControl::createLabel: Success!" << std::endl;
    return true;
}

// 初始化静态成员
int LabelControl::Impl::s_nextId = 2000;

} // namespace UI
} // namespace LuaUI