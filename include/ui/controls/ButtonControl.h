/**
 * @file ButtonControl.h
 * @brief 按钮控件
 * @details 按钮控件，支持点击事件
 * @version 1.0.0
 */

#ifndef LUAUI_BUTTONCONTROL_H
#define LUAUI_BUTTONCONTROL_H

#include "ui/controls/BaseControl.h"
#include <afxwin.h>

namespace LuaUI {
namespace UI {

// 前向声明
class ButtonControl;

/**
 * @brief 按钮控件类
 */
class ButtonControl : public BaseControl {
public:
    ButtonControl();
    virtual ~ButtonControl();
    
    virtual std::string getType() const override;
    virtual bool createFromXml(Xml::XmlElement* xmlElement, CWnd* parent = nullptr) override;
    virtual void bindLuaFunctions(lua_State* lua) override;
    virtual CWnd* getWindow() override;
    virtual const CWnd* getWindow() const override;
    
    void setText(const std::string& text);
    std::string getText() const;
    
    static BaseControl* createInstance();
    bool createButton(CWnd* parent);

private:
    class Impl;
    Impl* m_impl;
};

} // namespace UI
} // namespace LuaUI

#endif // LUAUI_BUTTONCONTROL_H
