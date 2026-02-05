/**
 * @file LabelControl.cpp
 * @brief 标签控件实现
 */

#include "ui/controls/LabelControl.h"
#include <afxwin.h> // MFC support

namespace LuaUI {
namespace UI {

class LabelControl::Impl {
public:
    CStatic* label; ///< MFC标签控件指针
    std::string text; ///< 标签文本

    Impl() : label(nullptr), text("Label") { }
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

        // 创建MFC标签
        createLabel(parent);
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
    if (!parent || m_impl->label) {
        return false; // 父窗口不存在或标签已存在
    }

    // 创建MFC标签控件
    m_impl->label = new CStatic();
    
    // 创建标签
    CString text = CString(m_impl->text.c_str());
    if (!m_impl->label->Create(text, WS_CHILD | WS_VISIBLE | SS_LEFT,
                              CRect(m_x, m_y, m_x + m_width, m_y + m_height),
                              parent, AFX_IDW_PANE_FIRST + 3)) {  // 使用唯一ID
        delete m_impl->label;
        m_impl->label = nullptr;
        return false;
    }

    // 显示标签
    m_impl->label->ShowWindow(SW_SHOW);
    m_impl->label->UpdateWindow();

    return true;
}

} // namespace UI
} // namespace LuaUI