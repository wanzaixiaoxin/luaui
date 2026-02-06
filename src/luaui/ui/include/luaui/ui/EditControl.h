/**
 * @file EditControl.h
 * @brief 文本框控件
 * @details 用于输入和编辑文本的控件
 * @version 1.0.0
 */

#ifndef LUAUI_EDITCONTROL_H
#define LUAUI_EDITCONTROL_H

#include "ui/controls/BaseControl.h"

namespace LuaUI {
namespace UI {

/**
 * @brief 文本框控件类
 */
class EditControl : public BaseControl {
public:
    EditControl();
    virtual ~EditControl();
    
    virtual std::string getType() const override;
    virtual bool createFromXml(Xml::XmlElement* xmlElement, CWnd* parent = nullptr) override;
    virtual void bindLuaFunctions(lua_State* lua) override;
    virtual CWnd* getWindow() override;
    virtual const CWnd* getWindow() const override;
    
    /**
     * @brief 设置文本
     * @param text 文本
     */
    void setText(const std::string& text);
    
    /**
     * @brief 获取文本
     * @return 文本
     */
    std::string getText() const;
    
    /**
     * @brief 静态创建函数
     * @return 文本框控件指针
     */
    static BaseControl* createInstance();

private:
    class Impl;
    Impl* m_impl;
};

} // namespace UI
} // namespace LuaUI

#endif // LUAUI_EDITCONTROL_H
