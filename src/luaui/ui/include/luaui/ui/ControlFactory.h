/**
 * @file ControlFactory.h
 * @brief 控件工厂类
 * @details 根据XML元素类型创建对应的控件对象
 * @version 1.0.0
 */

#ifndef LUAUI_CONTROLFACTORY_H
#define LUAUI_CONTROLFACTORY_H

#include <string>
#include <map>
#include <memory>
#include "ui/controls/BaseControl.h"
#include "xml/parser/XmlParser.h"

// MFC forward declarations
class CWnd;

namespace LuaUI {
namespace UI {

/**
 * @brief 控件创建函数类型
 * @details 用于创建控件的函数指针类型
 */
typedef BaseControl* (*ControlCreator)();

/**
 * @brief 控件工厂类
 * @details 单例模式，管理所有控件类型的注册和创建
 */
class ControlFactory {
public:
    /**
     * @brief 获取工厂单例
     * @return 工厂引用
     */
    static ControlFactory& instance();
    
    /**
     * @brief 创建控件
     * @param type 控件类型
     * @return 控件指针，如果类型不存在返回nullptr
     */
    BaseControl* createControl(const std::string& type);
    
    /**
     * @brief 从XML创建控件
     * @param xmlElement XML元素
     * @param parent 父窗口
     * @return 控件指针，如果创建失败返回nullptr
     */
    BaseControl* createFromXml(Xml::XmlElement* xmlElement, CWnd* parent = nullptr);
    
    /**
     * @brief 注册控件类型
     * @param type 控件类型名称
     * @param creator 控件创建函数
     */
    void registerControl(const std::string& type, ControlCreator creator);
    
    /**
     * @brief 检查控件类型是否已注册
     * @param type 控件类型名称
     * @return 已注册返回true，否则返回false
     */
    bool isRegistered(const std::string& type) const;
    
    /**
     * @brief 获取所有已注册的控件类型
     * @return 控件类型列表
     */
    std::vector<std::string> getRegisteredTypes() const;
    
    /**
     * @brief 注销控件类型
     * @param type 控件类型名称
     * @return 成功返回true，失败返回false
     */
    bool unregisterControl(const std::string& type);
    
private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    ControlFactory();
    
    /**
     * @brief 私有析构函数
     */
    ~ControlFactory();
    
    /**
     * @brief 禁止拷贝构造
     */
    ControlFactory(const ControlFactory&);
    
    /**
     * @brief 禁止赋值操作
     */
    ControlFactory& operator=(const ControlFactory&);
    
    /**
     * @brief 注册所有内置控件类型
     */
    void registerBuiltinControls();
    
    std::map<std::string, ControlCreator> m_creators; ///< 控件创建函数映射表
};

/**
 * @brief 控件注册辅助类
 * @details 用于自动注册控件类型
 */
template<typename T>
class ControlRegistrar {
public:
    /**
     * @brief 构造函数，自动注册控件类型
     * @param type 控件类型名称
     */
    ControlRegistrar(const std::string& type) {
        ControlFactory::instance().registerControl(type, &T::createInstance);
    }
};

// 控件注册宏
#define REGISTER_CONTROL(type, className) \
    static LuaUI::UI::ControlRegistrar<className> s_##type##_registrar(#type);

} // namespace UI
} // namespace LuaUI

#endif // LUAUI_CONTROLFACTORY_H
