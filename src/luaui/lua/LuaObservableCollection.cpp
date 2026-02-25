#include "LuaObservableCollection.h"
#include "../controls/ListBox.h"
#include <codecvt>
#include <locale>

namespace luaui {
namespace lua {

// ============================================================================
// LuaObservableCollection 实现
// ============================================================================
LuaObservableCollection::LuaObservableCollection(lua_State* L, int tableIndex)
    : m_L(L)
    , m_tableRef(LUA_NOREF)
{
    // 确保是表类型
    if (!lua_istable(L, tableIndex)) {
        utils::Logger::Error("[LuaObservableCollection] Expected table, got other type");
        return;
    }
    
    // 复制表到栈顶并创建引用
    lua_pushvalue(L, tableIndex);
    m_tableRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    utils::Logger::InfoF("[LuaObservableCollection] Created with ref %d", m_tableRef);
}

LuaObservableCollection::~LuaObservableCollection() {
    if (m_tableRef != LUA_NOREF && m_L) {
        luaL_unref(m_L, LUA_REGISTRYINDEX, m_tableRef);
        utils::Logger::Info("[LuaObservableCollection] Destroyed");
    }
}

size_t LuaObservableCollection::GetCount() const {
    if (m_tableRef == LUA_NOREF || !m_L) return 0;
    
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableRef);
    size_t count = lua_rawlen(m_L, -1);
    lua_pop(m_L, 1);
    return count;
}

std::wstring LuaObservableCollection::GetItemDisplayText(size_t index) const {
    if (m_tableRef == LUA_NOREF || !m_L) return L"";
    
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableRef);
    
    // Lua 数组是 1-based
    lua_rawgeti(m_L, -1, static_cast<int>(index) + 1);
    
    std::wstring result = GetDisplayTextFromItem(-1);
    
    lua_pop(m_L, 2);  // pop item and table
    return result;
}

std::wstring LuaObservableCollection::GetDisplayTextFromItem(int itemIndex) const {
    if (!m_L) return L"";
    
    int idx = itemIndex < 0 ? lua_gettop(m_L) : itemIndex;
    
    // 如果是字符串，直接使用
    if (lua_isstring(m_L, idx)) {
        const char* str = lua_tostring(m_L, idx);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }
    
    // 如果是表且有 DisplayMemberPath，使用该字段
    if (lua_istable(m_L, idx) && !m_displayMemberPath.empty()) {
        lua_getfield(m_L, idx, m_displayMemberPath.c_str());
        if (lua_isstring(m_L, -1)) {
            const char* str = lua_tostring(m_L, -1);
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring result = converter.from_bytes(str);
            lua_pop(m_L, 1);
            return result;
        }
        lua_pop(m_L, 1);
    }
    
    // 如果是表且有 Name 字段，使用 Name
    if (lua_istable(m_L, idx)) {
        lua_getfield(m_L, idx, "Name");
        if (lua_isstring(m_L, -1)) {
            const char* str = lua_tostring(m_L, -1);
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring result = converter.from_bytes(str);
            lua_pop(m_L, 1);
            return result;
        }
        lua_pop(m_L, 1);
        
        // 尝试 Content 字段
        lua_getfield(m_L, idx, "Content");
        if (lua_isstring(m_L, -1)) {
            const char* str = lua_tostring(m_L, -1);
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring result = converter.from_bytes(str);
            lua_pop(m_L, 1);
            return result;
        }
        lua_pop(m_L, 1);
    }
    
    // 默认使用 tostring
    lua_getglobal(m_L, "tostring");
    lua_pushvalue(m_L, idx);
    lua_pcall(m_L, 1, 1, 0);
    const char* str = lua_tostring(m_L, -1);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring result = converter.from_bytes(str ? str : "[Item]");
    lua_pop(m_L, 1);
    
    return result;
}

std::vector<std::wstring> LuaObservableCollection::GetAllDisplayTexts() const {
    std::vector<std::wstring> result;
    size_t count = GetCount();
    result.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        result.push_back(GetItemDisplayText(i));
    }
    
    return result;
}

} // namespace lua
} // namespace luaui
