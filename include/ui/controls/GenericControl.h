/**
 * @file GenericControl.h
 * @brief 通用控件
 * @details 通用控件，用于在运行时动态创建未知类型的控件
 * @version 1.0.0
 */

#ifndef LUAUI_GENERICCONTROL_H
#define LUAUI_GENERICCONTROL_H

#include "ui/controls/BaseControl.h"

namespace LuaUI {
namespace UI {

/**
 * @brief 通用控件类
 */
class GenericControl : public BaseControl {
public:
    GenericControl();
    virtual ~GenericControl();
    
    virtual std::string getType() const override;
    virtual bool createFromXml(Xml::XmlElement* xmlElement, CWnd* parent = nullptr) override;
    virtual CWnd* getWindow() override;
    virtual const CWnd* getWindow() const override;
    
    /**
     * @brief 静态创建函数
     * @return 通用控件指针
     */
    static BaseControl* createInstance();

private:
    class Impl;
    Impl* m_impl;
};

} // namespace UI
} // namespace LuaUI

#endif // LUAUI_GENERICCONTROL_H