/**
 * @file LabelControl.cpp
 * @brief 标签控件实现
 */

#include "ui/controls/LabelControl.h"
#include "ui/controls/BaseControl.h"
#include "utils/Logger.h"
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

        // 不在XML解析时创建MFC标签，延迟到LayoutEngine::showUI中统一创建
        // 这样可以确保父窗口完全初始化后再创建子控件
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

void LabelControl::setProperty(const std::string& name, const std::string& value) {
    // Call base class implementation
    BaseControl::setProperty(name, value);
    
    // Handle text property
    if (name == "text") {
        setText(value);
    }
}

std::string LabelControl::getProperty(const std::string& name) const {
    // Handle text property
    if (name == "text") {
        return getText();
    }
    
    // Call base class implementation
    return BaseControl::getProperty(name);
}

bool LabelControl::createLabel(CWnd* parent) {
    LOG_S_DEBUG_CAT("LabelControl") << "parent=" << parent 
              << ", existing label=" << m_impl->label;
    if (!parent || m_impl->label) {
        LOG_S_WARN_CAT("LabelControl") << "Failed - "
                  << (!parent ? "no parent" : "label already exists");
        return false; // 父窗口不存在或标签已存在
    }

    // 创建MFC标签控件
    m_impl->label = new CStatic();
    
    // 创建标签，使用唯一 ID
    CString text = CString(m_impl->text.c_str());
    int controlId = m_impl->getNextId();
    LOG_S_DEBUG_CAT("LabelControl") << "Creating with ID=" << controlId;
    // 使用 SS_NOTIFY 接收鼠标事件，添加 WS_BORDER 便于调试查看边界
    if (!m_impl->label->Create(text, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_LEFT | SS_NOTIFY,
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

    LOG_INFO_CAT("LabelControl", "Success!");
    return true;
}

// 初始化静态成员
int LabelControl::Impl::s_nextId = 2000;

} // namespace UI
} // namespace LuaUI