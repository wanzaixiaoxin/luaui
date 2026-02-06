/**
 * @file WindowControl.cpp
 * @brief 窗口控件实现
 */

#include "ui/controls/WindowControl.h"
#include "ui/controls/ButtonControl.h"
#include "ui/controls/LabelControl.h"
#include "xml/parser/XmlParser.h"
#include "utils/Logger.h"
#include <afxwin.h> // MFC support
#include <iostream>

namespace LuaUI {

namespace UI {

// LuaUIWindow 的消息映射实现
IMPLEMENT_DYNCREATE(LuaUIWindow, CWnd)

BEGIN_MESSAGE_MAP(LuaUIWindow, CWnd)
    ON_WM_CLOSE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

LuaUIWindow::LuaUIWindow()
    : m_owner(nullptr)
{
}

LuaUIWindow::~LuaUIWindow()
{
}

class WindowControl::Impl {
public:
    LuaUIWindow* window;
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
    // 清理子控件
    for (std::vector<BaseControl*>::iterator it = m_children.begin();
         it != m_children.end(); ++it) {
        delete *it;
    }
    m_children.clear();
    
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

bool WindowControl::addChild(BaseControl* child) {
    if (!child) {
        return false;
    }
    
    // 检查是否已存在
    for (std::vector<BaseControl*>::iterator it = m_children.begin();
         it != m_children.end(); ++it) {
        if (*it == child) {
            return false;
        }
    }
    
    m_children.push_back(child);
    return true;
}

BaseControl* WindowControl::findChildById(const std::string& id) {
    for (std::vector<BaseControl*>::iterator it = m_children.begin();
         it != m_children.end(); ++it) {
        if ((*it)->getId() == id) {
            return *it;
        }
        
        // 递归查找子控件的子控件
        BaseControl* found = (*it)->findChildById(id);
        if (found) {
            return found;
        }
    }
    return nullptr;
}

void WindowControl::createChildWindows() {
    if (!m_impl->window) {
        LOG_ERROR("WindowControl::createChildWindows: No window available!");
        return;
    }
    
    // 获取客户区矩形，用于计算子控件相对位置
    CRect clientRect;
    m_impl->window->GetClientRect(&clientRect);
    LOG_S_DEBUG_CAT("WindowControl") << "Client rect = " 
              << clientRect.left << "," << clientRect.top << " - " 
              << clientRect.Width() << "x" << clientRect.Height();
    
    // 递归创建所有子控件的MFC窗口
    for (std::vector<BaseControl*>::iterator it = m_children.begin();
         it != m_children.end(); ++it) {
        BaseControl* child = *it;
        std::string type = child->getType();
        
        LOG_S_DEBUG_CAT("WindowControl") << "Creating child window: " << type << " (id: " << child->getId() << ")";
        
        // 根据控件类型创建MFC控件
        if (type == "button") {
            ButtonControl* button = dynamic_cast<ButtonControl*>(child);
            if (button) {
                button->createButton(m_impl->window);
            }
        } else if (type == "label") {
            LabelControl* label = dynamic_cast<LabelControl*>(child);
            if (label) {
                label->createLabel(m_impl->window);
            }
        }
        // 其他控件类型可以在此处扩展
        
        // 递归创建子控件的子控件（如果有）
        if (child->getType() != "button" && child->getType() != "label") {
            // 对于容器类控件，需要递归创建其子控件
            WindowControl* windowChild = dynamic_cast<WindowControl*>(child);
            if (windowChild) {
                windowChild->createChildWindows();
            }
        }
    }
    
    // 强制重绘窗口以显示所有子控件
    m_impl->window->Invalidate();
    m_impl->window->UpdateWindow();
    
    // 确保所有子控件都被正确显示（某些情况下需要额外处理）
    for (std::vector<BaseControl*>::iterator it = m_children.begin();
         it != m_children.end(); ++it) {
        CWnd* childWnd = (*it)->getWindow();
        if (childWnd && ::IsWindow(childWnd->m_hWnd)) {
            childWnd->Invalidate();
            childWnd->UpdateWindow();
        }
    }
}

bool WindowControl::createWindow(CWnd* parent) {
    if (m_impl->window) {
        return true;
    }

    m_impl->window = new LuaUIWindow();
    m_impl->window->setOwnerControl(this);

    CString title = CString(m_impl->title.c_str());
    
    // 注册自定义窗口类
    static LPCTSTR className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1),
        NULL);
    
    // 使用 WS_CLIPCHILDREN 确保子控件正确显示
    // 父窗口为 NULL 表示顶级窗口
    if (!m_impl->window->CreateEx(
            0,
            className,
            title,
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            m_x, m_y, m_width, m_height,
            parent ? parent->m_hWnd : NULL,
            NULL)) {
        delete m_impl->window;
        m_impl->window = nullptr;
        LOG_ERROR("WindowControl::createWindow: Failed to create window!");
        return false;
    }

    LOG_INFO_CAT("WindowControl", "Window created successfully");
    
    // 不在这里显示窗口，等待所有子控件创建完成后再显示
    // m_impl->window->ShowWindow(SW_SHOW);
    // m_impl->window->UpdateWindow();

    return true;
}

} // namespace UI
} // namespace LuaUI