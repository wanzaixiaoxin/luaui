#pragma once

#include "mvvm/INotifyCollectionChanged.h"
#include "utils/Logger.h"
#include <vector>
#include <functional>
#include <string>
#include <memory>

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
// LuaCollectionChangedListener - Lua 集合变更监听器
// 通过 Lua 回调函数监听集合变更
// ============================================================================
class LuaCollectionChangedListener {
public:
    LuaCollectionChangedListener(lua_State* L, int tableRef);
    ~LuaCollectionChangedListener();
    
    // 设置 C++ 端回调
    using CollectionChangedCallback = std::function<void(mvvm::NotifyCollectionChangedAction, 
                                                          int index, 
                                                          const std::wstring& displayText)>;
    void SetCallback(CollectionChangedCallback callback) { m_callback = callback; }
    
    // 注册 Lua 监听器
    void RegisterListener();
    
    // 注销监听器
    void UnregisterListener();
    
    // 手动触发刷新（整个集合）
    void NotifyReset();

private:
    lua_State* m_L;
    int m_tableRef;       // 集合表的引用
    int m_listenerRef;    // Lua 监听器的引用
    CollectionChangedCallback m_callback;
    bool m_registered = false;
    
    // Lua 回调函数（静态，供 Lua 调用）
    static int LuaCallback(lua_State* L);
};

// ============================================================================
// LuaObservableCollection - Lua 集合的 C++ 包装（支持增量更新）
// ============================================================================
class LuaObservableCollection : public mvvm::INotifyCollectionChanged,
                                 public std::enable_shared_from_this<LuaObservableCollection> {
public:
    LuaObservableCollection(lua_State* L, int tableIndex);
    ~LuaObservableCollection();
    
    // INotifyCollectionChanged 实现
    void SubscribeCollectionChanged(mvvm::INotifyCollectionChanged::CollectionChangedHandler handler) override;
    void UnsubscribeCollectionChanged(mvvm::INotifyCollectionChanged::CollectionChangedHandler handler) override;
    
    // 获取集合大小
    size_t GetCount() const;
    
    // 获取指定索引的项（作为字符串显示）
    std::wstring GetItemDisplayText(size_t index) const;
    
    // 获取所有项的显示文本
    std::vector<std::wstring> GetAllDisplayTexts() const;

    // 设置显示字段名（用于复杂对象）
    void SetDisplayMemberPath(const std::string& path) { m_displayMemberPath = path; }
    
    // 获取原始 Lua 状态
    lua_State* GetLuaState() const { return m_L; }
    int GetTableRef() const { return m_tableRef; }
    
    // 启用/禁用增量更新监听
    void EnableIncrementalUpdates(bool enable);
    bool IsIncrementalUpdatesEnabled() const { return m_listener != nullptr; }

private:
    lua_State* m_L;
    int m_tableRef;  // Lua registry reference to the table
    std::string m_displayMemberPath;
    
    std::unique_ptr<LuaCollectionChangedListener> m_listener;
    std::vector<mvvm::INotifyCollectionChanged::CollectionChangedHandler> m_handlers;
    
    // 获取项的显示文本
    std::wstring GetDisplayTextFromItem(int itemIndex) const;
    
    // 从 Lua 表获取字符串值
    std::wstring GetLuaString(int index, const char* field = nullptr) const;
    
    // 处理集合变更
    void OnCollectionChanged(const mvvm::NotifyCollectionChangedEventArgs& args);
    
    // 从 Lua 回调接收变更
    void HandleLuaChange(const std::string& action, int index, const std::wstring& text);
    
    friend class LuaCollectionChangedListener;
};

// ============================================================================
// ObservableCollectionBinding - ListBox 与 ObservableCollection 的增量绑定
// ============================================================================
class ObservableCollectionBinding {
public:
    ObservableCollectionBinding(
        std::shared_ptr<luaui::controls::ListBox> listBox,
        std::shared_ptr<LuaObservableCollection> collection
    );

    ~ObservableCollectionBinding();

    void Detach();
    void SyncAllItems();
    void InsertItem(int index, const std::wstring& text);
    void RemoveItem(int index);
    void MoveItem(int oldIndex, int newIndex);
    void ReplaceItem(int index, const std::wstring& text);

private:
    void OnCollectionChanged(const mvvm::NotifyCollectionChangedEventArgs& args);

    std::shared_ptr<luaui::controls::ListBox> m_listBox;
    std::shared_ptr<LuaObservableCollection> m_collection;
    mvvm::INotifyCollectionChanged::CollectionChangedHandler m_handler;
    bool m_attached = true;
};

} // namespace lua
} // namespace luaui
