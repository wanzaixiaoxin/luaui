/**
 * @file WindowControl.h
 * @brief 窗口控件
 * @details 主窗口控件，作为其他控件的容器
 * @version 1.0.0
 */

#ifndef LUAUI_WINDOWCONTROL_H
#define LUAUI_WINDOWCONTROL_H

#include "ui/controls/BaseControl.h"
#include <afxwin.h>
#include <vector>

namespace LuaUI {
namespace UI {

// 前向声明
class WindowControl;

/**
 * @brief MFC 窗口类，用于处理消息
 */
class LuaUIWindow : public CFrameWnd {
    DECLARE_DYNCREATE(LuaUIWindow)

public:
    LuaUIWindow();
    virtual ~LuaUIWindow();

    /**
     * @brief 设置所有者控件
     * @param owner 所有者控件
     */
    void setOwnerControl(WindowControl* owner) { m_owner = owner; }

    /**
     * @brief 获取所有者控件
     * @return 所有者控件
     */
    WindowControl* getOwnerControl() const { return m_owner; }

protected:
    DECLARE_MESSAGE_MAP()

private:
    WindowControl* m_owner; ///< 所有者控件
};

/**
 * @brief 窗口控件类
 */
class WindowControl : public BaseControl {
public:
    WindowControl();
    virtual ~WindowControl();
    
    virtual std::string getType() const override;
    virtual bool createFromXml(Xml::XmlElement* xmlElement, CWnd* parent = nullptr) override;
    virtual void bindLuaFunctions(lua_State* lua) override;
    virtual CWnd* getWindow() override;
    virtual const CWnd* getWindow() const override;
    
    /**
     * @brief 设置窗口标题
     * @param title 标题
     */
    void setTitle(const std::string& title);
    
    /**
     * @brief 获取窗口标题
     * @return 标题
     */
    std::string getTitle() const;
    
    /**
/**
     * @brief 静态创建函数
     * @return 窗口控件指针
     */
    static BaseControl* createInstance();

    /**
     * @brief 创建MFC窗口
     * @param parent 父窗口
     * @return 成功返回true，失败返回false
     */
    bool createWindow(CWnd* parent);

    /**
     * @brief 添加子控件
     * @param child 子控件
     * @return 成功返回true，失败返回false
     */
    bool addChild(BaseControl* child) override;

    /**
     * @brief 根据ID查找子控件
     * @param id 控件ID
     * @return 找到返回控件指针，否则返回nullptr
     */
    BaseControl* findChildById(const std::string& id) override;

    /**
     * @brief 创建所有子控件的MFC窗口
     * @details 在主窗口创建后，递归创建所有子控件的MFC窗口
     */
    void createChildWindows();

private:
    class Impl;
    Impl* m_impl;
    std::vector<BaseControl*> m_children; ///< 子控件列表

    friend class LuaUIWindow; // 允许窗口类访问私有成员
};

} // namespace UI
} // namespace LuaUI

#endif // LUAUI_WINDOWCONTROL_H
