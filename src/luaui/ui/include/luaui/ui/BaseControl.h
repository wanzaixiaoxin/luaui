/**
 * @file BaseControl.h
 * @brief 控件基类
 * @details 所有UI控件的基类，提供通用的属性和事件处理
 * @version 1.0.0
 */

#ifndef LUAUI_BASECONTROL_H
#define LUAUI_BASECONTROL_H

#include <string>
#include <map>
#include <memory>
#include "xml/parser/XmlParser.h"
#include "ILuaUI.h"  // 包含接口定义

// MFC forward declarations
class CWnd;

// Lua forward declaration
struct lua_State;

namespace LuaUI {
namespace UI {

/**
 * @brief Lua函数引用
 * @details 用于存储Lua回调函数的引用
 */
struct LuaFunctionRef {
    lua_State* lua;     ///< Lua状态
    int ref;            ///< 引用ID
    
    /**
     * @brief 构造函数
     */
    LuaFunctionRef()
        : lua(nullptr)
        , ref(-1)
    {
    }
    
    /**
     * @brief 构造函数
     * @param L Lua状态
     * @param r 引用ID
     */
    LuaFunctionRef(lua_State* L, int r)
        : lua(L)
        , ref(r)
    {
    }
};

/**
 * @brief 控件基类
 * @details 提供所有UI控件的通用接口和实现
 */
class BaseControl : public IControl {
public:
    /**
     * @brief 构造函数
     */
    BaseControl();
    
    /**
     * @brief 析构函数
     */
    virtual ~BaseControl();
    
    /**
     * @brief 创建实例的静态方法
     * @return 新的控件实例
     */
    static BaseControl* createInstance();
    
    /**
     * @brief 从XML创建控件
     * @param xmlElement XML元素
     * @param parent 父窗口（MFC窗口）
     * @return 成功返回true，失败返回false
     */
    virtual bool createFromXml(Xml::XmlElement* xmlElement, CWnd* parent = nullptr);
    
    /**
     * @brief 绑定Lua函数
     * @param lua Lua状态
     */
    virtual void bindLuaFunctions(lua_State* lua);
    
    /**
     * @brief 获取控件ID
     * @return 控件ID
     */
    const std::string& getId() const;
    
    /**
     * @brief 设置控件ID
     * @param id 控件ID
     */
    void setId(const std::string& id);
    
    /**
     * @brief 获取控件类型
     * @return 控件类型名称
     */
    virtual std::string getType() const = 0;
    
    /**
     * @brief 设置属性
     * @param name 属性名
     * @param value 属性值
     */
    virtual void setProperty(const std::string& name, const std::string& value);
    
    /**
     * @brief 获取属性
     * @param name 属性名
     * @return 属性值
     */
    virtual std::string getProperty(const std::string& name) const;
    
    /**
     * @brief 设置事件处理器
     * @param eventName 事件名称
     * @param handler Lua函数引用
     */
    virtual void setEventHandler(const std::string& eventName, const LuaFunctionRef& handler);
    
    /**
     * @brief 触发事件
     * @param eventName 事件名称
     * @return 成功返回true，失败返回false
     */
    virtual bool fireEvent(const std::string& eventName);
    
    /**
     * @brief 显示控件
     */
    virtual void show();
    
    /**
     * @brief 隐藏控件
     */
    virtual void hide();
    
    /**
     * @brief 启用控件
     */
    virtual void enable();
    
    /**
     * @brief 禁用控件
     */
    virtual void disable();

    /**
     * @brief 获取X坐标
     * @return X坐标
     */
    int getX() const { return m_x; }

    /**
     * @brief 获取Y坐标
     * @return Y坐标
     */
    int getY() const { return m_y; }

    /**
     * @brief 获取宽度
     * @return 宽度
     */
    int getWidth() const { return m_width; }

    /**
     * @brief 获取高度
     * @return 高度
     */
    int getHeight() const { return m_height; }

    /**
     * @brief 设置位置
     * @param x X坐标
     * @param y Y坐标
     */
    virtual void setPosition(int x, int y);

    /**
     * @brief 设置大小
     * @param width 宽度
     * @param height 高度
     */
    virtual void setSize(int width, int height);
    
    /**
     * @brief 获取MFC窗口对象
     * @return MFC窗口指针
     */
    virtual CWnd* getWindow() = 0;
    
    /**
     * @brief 获取MFC窗口对象（const版本）
     * @return MFC窗口指针
     */
    virtual const CWnd* getWindow() const = 0;
    
    /**
     * @brief 添加子控件
     * @param child 子控件
     * @return 成功返回true，失败返回false
     */
    virtual bool addChild(BaseControl* child);
    
    /**
     * @brief 根据ID查找子控件
     * @param id 控件ID
     * @return 找到返回控件指针，否则返回nullptr
     */
    virtual BaseControl* findChildById(const std::string& id);
    
    /**
     * @brief 调用Lua事件处理器
     * @param handler Lua函数引用
     * @return 成功返回true，失败返回false
     */
    bool callLuaHandler(const LuaFunctionRef& handler);
    
protected:
    std::string m_id;                                      ///< 控件ID
    std::map<std::string, std::string> m_properties;      ///< 属性映射表
    std::map<std::string, LuaFunctionRef> m_eventHandlers; ///< 事件处理器映射表
    int m_x;                                              ///< X坐标
    int m_y;                                              ///< Y坐标
    int m_width;                                          ///< 宽度
    int m_height;                                         ///< 高度
    bool m_visible;                                       ///< 是否可见
    bool m_enabled;                                       ///< 是否启用
    
    /**
     * @brief 解析通用属性
     * @param xmlElement XML元素
     */
    void parseCommonAttributes(Xml::XmlElement* xmlElement);
    
    /**
     * @brief 解析位置和大小
     * @param xmlElement XML元素
     */
    void parseGeometry(Xml::XmlElement* xmlElement);
};

} // namespace UI
} // namespace LuaUI

#endif // LUAUI_BASECONTROL_H
