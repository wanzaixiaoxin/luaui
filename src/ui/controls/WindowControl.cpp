/**
 * @file WindowControl.cpp
 * @brief 窗口控件实现
 */

#include "ui/controls/WindowControl.h"
#include <afxwin.h> // MFC support

namespace LuaUI {
namespace UI {

class WindowControl::Impl {
public:
    CFrameWnd* window;
    std::string title;

    Impl() : window(nullptr), title("LuaUI Window") { }
    ~Impl() {
        if (window) {
            window->DestroyWindow();
            window = nullptr;
        }
    }
};

WindowControl::WindowControl()
    : m_impl(new Impl())
{
}

WindowControl::~WindowControl() {
    delete m_impl;
}

std::string WindowControl::getType() const {
    return "window";
}

bool WindowControl::createFromXml(Xml::XmlElement* xmlElement, CWnd* parent) {
    bool result = BaseControl::createFromXml(xmlElement, parent);

    if (xmlElement) {
        std::string title = xmlElement->getAttribute("title");
        if (!title.empty()) {
            setTitle(title);
        }
    }

    return result;
}

void WindowControl::bindLuaFunctions(lua_State* /*lua*/) {
}

CWnd* WindowControl::getWindow() {
    return m_impl->window;
}

const CWnd* WindowControl::getWindow() const {
    return m_impl->window;
}

void WindowControl::setTitle(const std::string& title) {
    m_impl->title = title;
    if (m_impl->window) {
        m_impl->window->SetWindowTextA(title.c_str());
    }
}

std::string WindowControl::getTitle() const {
    return m_impl->title;
}

BaseControl* WindowControl::createInstance() {
    return new WindowControl();
}

bool WindowControl::createWindow(CWnd* parent) {
    if (m_impl->window) {
        return true;
    }

    m_impl->window = new CFrameWnd();
    
    CString title = CString(m_impl->title.c_str());
    if (!m_impl->window->Create(NULL, title, WS_OVERLAPPEDWINDOW,
                               CRect(m_x, m_y, m_x + m_width, m_y + m_height), 
                               parent, NULL)) {
        delete m_impl->window;
        m_impl->window = nullptr;
        return false;
    }

    m_impl->window->ShowWindow(SW_SHOW);
    m_impl->window->UpdateWindow();

    return true;
}

} // namespace UI
} // namespace LuaUI