#pragma once

#include "mvvm/IBindable.h"
#include "utils/Logger.h"
#include <vector>
#include <functional>
#include <string>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace luaui {
namespace controls {
    class ListBox;
}

namespace lua {

// ============================================================================
// LuaObservableCollection - Lua 集合的 C++ 包装
// 提供对 Lua 表的只读访问，用于数据绑定
// ============================================================================
class LuaObservableCollection {
public:
    LuaObservableCollection(lua_State* L, int tableIndex);
    ~LuaObservableCollection();
    
    // 获取集合大小
    size_t GetCount() const;
    
    // 获取指定索引的项（作为字符串显示）
    std::wstring GetItemDisplayText(size_t index) const;
    
    // 获取所有项的显示文本
    std::vector<std::wstring> GetAllDisplayTexts() const;
    
    // 设置显示字段名（用于复杂对象）
    void SetDisplayMemberPath(const std::string& path) { m_displayMemberPath = path; }

private:
    lua_State* m_L;
    int m_tableRef;  // Lua registry reference to the table
    std::string m_displayMemberPath;
    
    // 获取项的显示文本
    std::wstring GetDisplayTextFromItem(int itemIndex) const;
    
    // 从 Lua 表获取字符串值
    std::wstring GetLuaString(int index, const char* field = nullptr) const;
};

} // namespace lua
} // namespace luaui
