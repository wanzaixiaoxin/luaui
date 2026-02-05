/**
 * @file ButtonControl.h
 * @brief 按钮控件
 * @details 按钮控件，支持点击事件
 * @version 1.0.0
 */

#ifndef LUAUI_BUTTONCONTROL_H
#define LUAUI_BUTTONCONTROL_H

#include "ui/controls/BaseControl.h"

namespace LuaUI {
namespace UI {

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
    
    /**
     * @brief 设置按钮文本
     * @param text 文本
     */
    void setText(const std::string& text);
    
    /**
     * @brief 获取按钮文本
     * @return 文本
     */
    std::string getText() const;
    
    /**
     * @brief 静态创建函数
     * @return 按钮控件指针
     */
    static BaseControl* createInstance();

    /**
     * @brief 创建MFC按钮
     * @param parent 父窗口
     * @return 成功返回true，失败返回false
     */
    bool createButton(CWnd* parent);

private:
    class Impl;
    Impl* m_impl;
};

} // namespace UI
} // namespace LuaUI

#endif // LUAUI_BUTTONCONTROL_H
