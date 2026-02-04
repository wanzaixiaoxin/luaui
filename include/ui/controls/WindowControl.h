/**
 * @file WindowControl.h
 * @brief 窗口控件
 * @details 主窗口控件，作为其他控件的容器
 * @version 1.0.0
 */

#ifndef LUAUI_WINDOWCONTROL_H
#define LUAUI_WINDOWCONTROL_H

#include "ui/controls/BaseControl.h"

namespace LuaUI {
namespace UI {

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
     * @brief 静态创建函数（供工厂使用）
     * @return 窗口控件指针
     */
    static BaseControl* createInstance();

private:
    class Impl;
    Impl* m_impl;
};

} // namespace UI
} // namespace LuaUI

#endif // LUAUI_WINDOWCONTROL_H
