/**
 * @file ILuaUI.h
 * @brief LuaUI接口定义
 * @details 定义LuaUI框架的核心接口
 * @version 1.0.0
 */

#ifndef ILUAUI_H
#define ILUAUI_H

#include <string>
#include <memory>

namespace LuaUI {

/**
 * @brief 控件接口
 * @details 所有UI控件的基接口
 */
class IControl {
public:
    virtual ~IControl() {}
    
    /**
     * @brief 获取控件ID
     * @return 控件ID字符串
     */
    virtual const std::string& getId() const = 0;
    
    /**
     * @brief 设置控件属性
     * @param name 属性名称
     * @param value 属性值
     */
    virtual void setProperty(const std::string& name, const std::string& value) = 0;
    
    /**
     * @brief 获取控件属性
     * @param name 属性名称
     * @return 属性值，如果不存在返回空字符串
     */
    virtual std::string getProperty(const std::string& name) const = 0;
    
    /**
     * @brief 显示控件
     */
    virtual void show() = 0;
    
    /**
     * @brief 隐藏控件
     */
    virtual void hide() = 0;
};

/**
 * @brief 布局引擎接口
 * @details XML布局解析和UI创建的接口
 */
class ILayoutEngine {
public:
    virtual ~ILayoutEngine() {}
    
    /**
     * @brief 从XML文件加载布局
     * @param xmlFile XML文件路径
     * @return 成功返回true，失败返回false
     */
    virtual bool loadFromXml(const std::string& xmlFile) = 0;
    
    /**
     * @brief 从XML字符串加载布局
     * @param xmlContent XML内容字符串
     * @return 成功返回true，失败返回false
     */
    virtual bool loadFromXmlString(const std::string& xmlContent) = 0;
    
    /**
     * @brief 根据ID获取控件
     * @param id 控件ID
     * @return 控件智能指针
     */
    virtual std::shared_ptr<IControl> getControl(const std::string& id) = 0;
    
    /**
     * @brief 显示UI
     * @details 显示所有创建的控件
     */
    virtual void showUI() = 0;
};

/**
 * @brief 脚本引擎接口
 * @details Lua脚本执行的接口
 */
class IScriptEngine {
public:
    virtual ~IScriptEngine() {}
    
    /**
     * @brief 加载并执行Lua脚本文件
     * @param luaFile Lua文件路径
     * @return 成功返回true，失败返回false
     */
    virtual bool loadScript(const std::string& luaFile) = 0;
    
    /**
     * @brief 执行Lua脚本字符串
     * @param script Lua脚本内容
     * @return 成功返回true，失败返回false
     */
    virtual bool executeString(const std::string& script) = 0;
    
    /**
     * @brief 调用Lua函数
     * @param funcName 函数名称
     * @return 成功返回true，失败返回false
     */
    virtual bool callFunction(const std::string& funcName) = 0;
};

/**
 * @brief 应用程序接口
 * @details LuaUI应用程序的主接口
 */
class IApplication {
public:
    virtual ~IApplication() {}
    
    /**
     * @brief 初始化应用程序
     * @return 成功返回true，失败返回false
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief 运行应用程序
     * @return 应用程序退出码
     */
    virtual int run() = 0;
    
    /**
     * @brief 退出应用程序
     */
    virtual void exit() = 0;
    
    /**
     * @brief 获取布局引擎
     * @return 布局引擎指针
     */
    virtual ILayoutEngine* getLayoutEngine() = 0;
    
    /**
     * @brief 获取脚本引擎
     * @return 脚本引擎指针
     */
    virtual IScriptEngine* getScriptEngine() = 0;
};

} // namespace LuaUI

#endif // ILUAUI_H
