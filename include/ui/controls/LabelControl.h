/**
 * @file LabelControl.h
 * @brief 标签控件
 * @details 用于显示静态文本的控件
 * @version 1.0.0
 */

#ifndef LUAUI_LABELCONTROL_H
#define LUAUI_LABELCONTROL_H

#include "ui/controls/BaseControl.h"

namespace LuaUI {
namespace UI {

/**
 * @brief 标签控件类
 */
class LabelControl : public BaseControl {
public:
    LabelControl();
    virtual ~LabelControl();
    
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
     * @return 标签控件指针
     */
    static BaseControl* createInstance();

private:
    class Impl;
    Impl* m_impl;
};

} // namespace UI
} // namespace LuaUI

#endif // LUAUI_LABELCONTROL_H
