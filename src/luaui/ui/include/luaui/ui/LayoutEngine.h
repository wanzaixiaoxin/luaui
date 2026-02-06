/**
 * @file LayoutEngine.h
 * @brief 布局引擎核心
 * @details 处理控件的位置计算和布局管理
 * @version 1.0.0
 */

#ifndef LUAUI_LAYOUTENGINE_H
#define LUAUI_LAYOUTENGINE_H

#include <string>
#include <vector>
#include <memory>
#include "xml/parser/XmlParser.h"
#include "ui/controls/BaseControl.h"
#include "ILuaUI.h"  // 包含接口定义

namespace LuaUI {
namespace Layout {

/**
 * @brief 布局类型
 */
enum LayoutType {
    LAYOUT_ABSOLUTE,    ///< 绝对定位
    LAYOUT_VERTICAL,     ///< 垂直布局
    LAYOUT_HORIZONTAL,   ///< 水平布局
    LAYOUT_GRID,        ///< 网格布局
    LAYOUT_FLEX         ///< 弹性布局
};

/**
 * @brief 边距信息
 */
struct Margins {
    int left;    ///< 左边距
    int top;     ///< 上边距
    int right;   ///< 右边距
    int bottom;  ///< 下边距
    
    /**
     * @brief 构造函数
     */
    Margins()
        : left(0)
        , top(0)
        , right(0)
        , bottom(0)
    {
    }
    
    /**
     * @brief 构造函数
     */
    Margins(int l, int t, int r, int b)
        : left(l)
        , top(t)
        , right(r)
        , bottom(b)
    {
    }
};

/**
 * @brief 尺寸信息
 */
struct Size {
    int width;   ///< 宽度
    int height;  ///< 高度
    
    /**
     * @brief 构造函数
     */
    Size()
        : width(0)
        , height(0)
    {
    }
    
    /**
     * @brief 构造函数
     */
    Size(int w, int h)
        : width(w)
        , height(h)
    {
    }
};

/**
 * @brief 位置信息
 */
struct Point {
    int x;  ///< X坐标
    int y;  ///< Y坐标
    
    /**
     * @brief 构造函数
     */
    Point()
        : x(0)
        , y(0)
    {
    }
    
    /**
     * @brief 构造函数
     */
    Point(int px, int py)
        : x(px)
        , y(py)
    {
    }
};

/**
 * @brief 矩形区域
 */
struct Rect {
    int x;      ///< X坐标
    int y;      ///< Y坐标
    int width;  ///< 宽度
    int height; ///< 高度
    
    /**
     * @brief 构造函数
     */
    Rect()
        : x(0)
        , y(0)
        , width(0)
        , height(0)
    {
    }
    
    /**
     * @brief 构造函数
     */
    Rect(int rx, int ry, int w, int h)
        : x(rx)
        , y(ry)
        , width(w)
        , height(h)
    {
    }
};

/**
 * @brief 布局引擎类
 */
class LayoutEngine : public ILayoutEngine {
public:
    /**
     * @brief 构造函数
     */
    LayoutEngine();
    
    /**
     * @brief 析构函数
     */
    ~LayoutEngine();
    
    /**
     * @brief 从XML布局创建控件树
     * @param xmlElement XML根元素
     * @return 成功返回true，失败返回false
     */
    bool createLayout(Xml::XmlElement* xmlElement);
    
    /**
     * @brief 计算布局
     * @details 根据布局类型计算所有控件的位置和大小
     * @return 成功返回true，失败返回false
     */
    bool calculateLayout();
    
    /**
     * @brief 获取根控件
     * @return 根控件指针
     */
    UI::BaseControl* getRootControl();
    
    /**
     * @brief 根据ID获取控件
     * @param id 控件ID
     * @return 控件指针，如果不存在返回nullptr
     */
    UI::BaseControl* getControlById(const std::string& id);
    
    /**
     * @brief 获取所有控件
     * @return 控件列表
     */
    std::vector<UI::BaseControl*> getAllControls();
    
    /**
     * @brief 更新布局
     * @details 当控件属性改变时重新计算布局
     * @return 成功返回true，失败返回false
     */
    bool updateLayout();
    
    /**
     * @brief 设置容器布局类型
     * @param containerId 容器ID
     * @param layoutType 布局类型
     * @return 成功返回true，失败返回false
     */
    bool setLayoutType(const std::string& containerId, LayoutType layoutType);
    
    /**
     * @brief 获取布局类型
     * @param containerId 容器ID
     * @return 布局类型
     */
    LayoutType getLayoutType(const std::string& containerId);
    
    /**
     * @brief 显示UI
     * @details 显示所有创建的控件
     */
    void showUI();
    
    /**
     * @brief 绑定Lua事件处理函数
     * @param scriptEngine 脚本引擎
     * @details 将Lua脚本中的事件函数绑定到控件
     */
    void bindLuaEvents(IScriptEngine* scriptEngine) override;
    
private:
    /**
     * @brief 递归绑定控件事件
     * @param control 控件
     * @param L Lua状态
     */
    void bindControlEvents(UI::BaseControl* control, struct lua_State* L);
    
    /**
     * @brief 递归显示所有控件
     * @param control 控件
     */
    void showAllControls(UI::BaseControl* control);
    
    // ILayoutEngine 接口实现
    virtual bool loadFromXml(const std::string& xmlFile) override;
    virtual bool loadFromXmlString(const std::string& xmlContent) override;
    virtual std::shared_ptr<IControl> getControl(const std::string& id) override;
    
private:
    UI::BaseControl* m_rootControl;               ///< 根控件
    std::map<std::string, UI::BaseControl*> m_controls; ///< 控件ID映射表
    std::map<std::string, LayoutType> m_layoutTypes;    ///< 布局类型映射表
    
    /**
     * @brief 递归创建控件树
     * @param xmlElement XML元素
     * @param parent 父控件
     * @return 创建的控件指针
     */
    UI::BaseControl* createControlTree(Xml::XmlElement* xmlElement, UI::BaseControl* parent);
    
    /**
     * @brief 递归计算布局
     * @param control 控件
     * @param rect 可用区域
     */
    void calculateControlLayout(UI::BaseControl* control, const Rect& rect);
    
    /**
     * @brief 计算绝对布局
     * @param control 控件
     * @param rect 可用区域
     */
    void calculateAbsoluteLayout(UI::BaseControl* control, const Rect& rect);
    
    /**
     * @brief 计算垂直布局
     * @param control 容器控件
     * @param rect 可用区域
     */
    void calculateVerticalLayout(UI::BaseControl* control, const Rect& rect);
    
    /**
     * @brief 计算水平布局
     * @param control 容器控件
     * @param rect 可用区域
     */
    void calculateHorizontalLayout(UI::BaseControl* control, const Rect& rect);
    
    /**
     * @brief 解析布局类型
     * @param layoutStr 布局字符串
     * @return 布局类型
     */
    LayoutType parseLayoutType(const std::string& layoutStr);
};

} // namespace Layout
} // namespace LuaUI

#endif // LUAUI_LAYOUTENGINE_H
