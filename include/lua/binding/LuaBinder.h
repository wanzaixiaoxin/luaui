/**
 * @file LuaBinder.h
 * @brief Lua绑定器
 * @details 提供Lua C++绑定的基础功能
 * @version 1.0.0
 */

#ifndef LUAUI_LUABINDER_H
#define LUAUI_LUABINDER_H

#include <string>
#include <map>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace LuaUI {
namespace Lua {
namespace Binding {

/**
 * @brief Lua绑定器类
 * @details 提供Lua和C++之间的数据交换和函数调用
 */
class LuaBinder {
public:
    /**
     * @brief 构造函数
     * @param lua Lua状态
     */
    LuaBinder(lua_State* lua);
    
    /**
     * @brief 析构函数
     */
    ~LuaBinder();
    
    /**
     * @brief 获取Lua状态
     * @return Lua状态指针
     */
    lua_State* getState() const;
    
    /**
     * @brief 注册函数到Lua全局命名空间
     * @param name 函数名
     * @param func C++函数指针
     */
    void registerFunction(const std::string& name, lua_CFunction func);
    
    /**
     * @brief 注册函数到Lua表
     * @param tableName 表名
     * @param funcName 函数名
     * @param func C++函数指针
     */
    void registerFunctionToTable(const std::string& tableName,
                                 const std::string& funcName,
                                 lua_CFunction func);
    
    /**
     * @brief 注册函数到Lua表的嵌套表
     * @param tableName 表名
     * @param subTableName 子表名
     * @param funcName 函数名
     * @param func C++函数指针
     */
    void registerFunctionToSubTable(const std::string& tableName,
                                   const std::string& subTableName,
                                   const std::string& funcName,
                                   lua_CFunction func);
    
    /**
     * @brief 注册表到Lua全局命名空间
     * @param tableName 表名
     */
    void registerTable(const std::string& tableName);
    
    /**
     * @brief 向Lua全局变量设置整数值
     * @param name 变量名
     * @param value 整数值
     */
    void setGlobalInt(const std::string& name, int value);
    
    /**
     * @brief 向Lua全局变量设置字符串值
     * @param name 变量名
     * @param value 字符串值
     */
    void setGlobalString(const std::string& name, const std::string& value);
    
    /**
     * @brief 向Lua全局变量设置布尔值
     * @param name 变量名
     * @param value 布尔值
     */
    void setGlobalBool(const std::string& name, bool value);

    /**
     * @brief 向Lua表字段设置整数值
     * @param tableName 表名
     * @param fieldName 字段名
     * @param value 整数值
     */
    void setTableInt(const std::string& tableName, const std::string& fieldName, int value);

    /**
     * @brief 向Lua表字段设置字符串值
     * @param tableName 表名
     * @param fieldName 字段名
     * @param value 字符串值
     */
    void setTableString(const std::string& tableName, const std::string& fieldName, const std::string& value);

    /**
     * @brief 从Lua全局变量获取整数值
     * @param name 变量名
     * @param defaultValue 默认值
     * @return 整数值
     */
    int getGlobalInt(const std::string& name, int defaultValue = 0);
    
    /**
     * @brief 从Lua全局变量获取字符串值
     * @param name 变量名
     * @param defaultValue 默认值
     * @return 字符串值
     */
    std::string getGlobalString(const std::string& name, const std::string& defaultValue = "");
    
    /**
     * @brief 从Lua全局变量获取布尔值
     * @param name 变量名
     * @param defaultValue 默认值
     * @return 布尔值
     */
    bool getGlobalBool(const std::string& name, bool defaultValue = false);
    
    /**
     * @brief 调用Lua函数
     * @param funcName 函数名
     * @param nargs 参数个数
     * @param nresults 返回值个数
     * @return 成功返回true，失败返回false
     */
    bool callFunction(const std::string& funcName, int nargs = 0, int nresults = 0);
    
    /**
     * @brief 执行Lua字符串
     * @param script Lua脚本
     * @return 成功返回true，失败返回false
     */
    bool doString(const std::string& script);
    
    /**
     * @brief 执行Lua文件
     * @param filename 文件名
     * @return 成功返回true，失败返回false
     */
    bool doFile(const std::string& filename);
    
    /**
     * @brief 获取栈顶的值作为字符串
     * @return 字符串值
     */
    std::string popString();
    
    /**
     * @brief 获取栈顶的值作为整数
     * @return 整数值
     */
    int popInt();
    
    /**
     * @brief 获取栈顶的值作为布尔值
     * @return 布尔值
     */
    bool popBool();
    
    /**
     * @brief 压入整数到栈
     * @param value 整数值
     */
    void pushInt(int value);
    
    /**
     * @brief 压入字符串到栈
     * @param value 字符串值
     */
    void pushString(const std::string& value);
    
    /**
     * @brief 压入布尔值到栈
     * @param value 布尔值
     */
    void pushBool(bool value);
    
    /**
     * @brief 压入nil到栈
     */
    void pushNil();
    
    /**
     * @brief 压入空表到栈
     */
    void pushNewTable();
    
    /**
     * @brief 设置表字段
     * @param key 键
     */
    void setField(const std::string& key);
    
    /**
     * @brief 获取表字段
     * @param key 键
     * @return 字段类型
     */
    int getField(const std::string& key);
    
    /**
     * @brief 打印Lua栈内容（调试用）
     */
    void printStack();

private:
    lua_State* m_lua;  ///< Lua状态
    
    /**
     * @brief 确保表存在
     * @param tableName 表名
     */
    void ensureTable(const std::string& tableName);
};

} // namespace Binding
} // namespace Lua
} // namespace LuaUI

#endif // LUAUI_LUABINDER_H
