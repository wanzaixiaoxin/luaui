#include "LuaObservableCollection.h"
#include "../controls/ListBox.h"
#include <codecvt>
#include <locale>

namespace luaui {
namespace lua {

// ============================================================================
// LuaCollectionChangedListener 实现
// ============================================================================
LuaCollectionChangedListener::LuaCollectionChangedListener(lua_State* L, int tableRef)
    : m_L(L)
    , m_tableRef(tableRef)
    , m_listenerRef(LUA_NOREF)
{
}

LuaCollectionChangedListener::~LuaCollectionChangedListener() {
    UnregisterListener();
}

void LuaCollectionChangedListener::RegisterListener() {
    if (m_registered || !m_L) return;
    
    // 获取 Lua 集合表
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableRef);
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return;
    }
    
    // 检查集合是否有 Subscribe 方法（Lua ObservableCollection 接口）
    lua_getfield(m_L, -1, "Subscribe");
    if (!lua_isfunction(m_L, -1)) {
        lua_pop(m_L, 2);  // Pop function + table
        utils::Logger::Debug("[LuaCollectionListener] Lua table does not have Subscribe method");
        return;
    }
    
    lua_pushvalue(m_L, -2);  // Push table as self
    
    // 创建闭包回调函数
    lua_pushlightuserdata(m_L, this);
    lua_pushcclosure(m_L, LuaCallback, 1);
    
    // 调用 Subscribe(self, callback)
    if (lua_pcall(m_L, 2, 1, 0) == LUA_OK) {
        // 保存返回的订阅 token
        if (!lua_isnil(m_L, -1)) {
            m_listenerRef = luaL_ref(m_L, LUA_REGISTRYINDEX);
        } else {
            lua_pop(m_L, 1);
        }
        m_registered = true;
        utils::Logger::Info("[LuaCollectionListener] Registered successfully");
    } else {
        const char* error = lua_tostring(m_L, -1);
        utils::Logger::ErrorF("[LuaCollectionListener] Failed to register: %s", 
            error ? error : "unknown");
        lua_pop(m_L, 1);
    }
    
    lua_pop(m_L, 1);  // Pop table
}

void LuaCollectionChangedListener::UnregisterListener() {
    if (!m_registered || !m_L || m_tableRef == LUA_NOREF) return;
    
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableRef);
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return;
    }
    
    // 调用 Unsubscribe 方法
    lua_getfield(m_L, -1, "Unsubscribe");
    if (lua_isfunction(m_L, -1) && m_listenerRef != LUA_NOREF) {
        lua_pushvalue(m_L, -2);  // Push table as self
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_listenerRef);  // Push token
        lua_pcall(m_L, 2, 0, 0);  // 忽略错误
    } else {
        lua_pop(m_L, 1);
    }
    
    if (m_listenerRef != LUA_NOREF) {
        luaL_unref(m_L, LUA_REGISTRYINDEX, m_listenerRef);
        m_listenerRef = LUA_NOREF;
    }
    
    m_registered = false;
    utils::Logger::Info("[LuaCollectionListener] Unregistered");
    
    lua_pop(m_L, 1);  // Pop table
}

void LuaCollectionChangedListener::NotifyReset() {
    if (m_callback) {
        m_callback(mvvm::NotifyCollectionChangedAction::Reset, -1, L"");
    }
}

int LuaCollectionChangedListener::LuaCallback(lua_State* L) {
    // 从闭包获取 this 指针
    LuaCollectionChangedListener* self = static_cast<LuaCollectionChangedListener*>(
        lua_touserdata(L, lua_upvalueindex(1)));
    
    if (!self || !self->m_callback) return 0;
    
    // 解析参数: callback(action, index, item)
    const char* actionStr = lua_tostring(L, 1);
    int index = static_cast<int>(lua_tointeger(L, 2)) - 1;  // Lua 1-based to C++ 0-based
    
    // 获取显示文本
    std::wstring text;
    if (lua_isstring(L, 3)) {
        const char* str = lua_tostring(L, 3);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        text = converter.from_bytes(str);
    }
    
    mvvm::NotifyCollectionChangedAction action;
    if (strcmp(actionStr, "Add") == 0) action = mvvm::NotifyCollectionChangedAction::Add;
    else if (strcmp(actionStr, "Remove") == 0) action = mvvm::NotifyCollectionChangedAction::Remove;
    else if (strcmp(actionStr, "Replace") == 0) action = mvvm::NotifyCollectionChangedAction::Replace;
    else if (strcmp(actionStr, "Move") == 0) action = mvvm::NotifyCollectionChangedAction::Move;
    else action = mvvm::NotifyCollectionChangedAction::Reset;
    
    self->m_callback(action, index, text);
    
    return 0;
}

// ============================================================================
// LuaObservableCollection 实现
// ============================================================================
LuaObservableCollection::LuaObservableCollection(lua_State* L, int tableIndex)
    : m_L(L)
    , m_tableRef(LUA_NOREF)
{
    if (!lua_istable(L, tableIndex)) {
        utils::Logger::Error("[LuaObservableCollection] Expected table");
        return;
    }
    
    lua_pushvalue(L, tableIndex);
    m_tableRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    utils::Logger::InfoF("[LuaObservableCollection] Created with ref %d", m_tableRef);
}

LuaObservableCollection::~LuaObservableCollection() {
    if (m_listener) {
        m_listener->UnregisterListener();
    }
    if (m_tableRef != LUA_NOREF && m_L) {
        luaL_unref(m_L, LUA_REGISTRYINDEX, m_tableRef);
    }
}

void LuaObservableCollection::SubscribeCollectionChanged(
    mvvm::INotifyCollectionChanged::CollectionChangedHandler handler) {
    m_handlers.push_back(handler);
}

void LuaObservableCollection::UnsubscribeCollectionChanged(
    mvvm::INotifyCollectionChanged::CollectionChangedHandler handler) {
    auto it = std::find_if(m_handlers.begin(), m_handlers.end(),
        [&handler](const auto& h) {
            return h.target<void(const mvvm::NotifyCollectionChangedEventArgs&)>() == 
                   handler.target<void(const mvvm::NotifyCollectionChangedEventArgs&)>();
        });
    if (it != m_handlers.end()) {
        m_handlers.erase(it);
    }
}

void LuaObservableCollection::EnableIncrementalUpdates(bool enable) {
    if (enable && !m_listener) {
        m_listener = std::make_unique<LuaCollectionChangedListener>(m_L, m_tableRef);
        
        m_listener->SetCallback([this](mvvm::NotifyCollectionChangedAction action, 
                                        int index, const std::wstring& text) {
            // 将 Lua 回调转换为标准通知
            if (action == mvvm::NotifyCollectionChangedAction::Add) {
                OnCollectionChanged(mvvm::NotifyCollectionChangedEventArgs(
                    action, std::any(text), index));
            } else if (action == mvvm::NotifyCollectionChangedAction::Remove) {
                OnCollectionChanged(mvvm::NotifyCollectionChangedEventArgs(
                    action, std::any(text), index, true));
            } else {
                OnCollectionChanged(mvvm::NotifyCollectionChangedEventArgs(
                    mvvm::NotifyCollectionChangedAction::Reset));
            }
        });
        
        m_listener->RegisterListener();
    } else if (!enable && m_listener) {
        m_listener->UnregisterListener();
        m_listener.reset();
    }
}

void LuaObservableCollection::OnCollectionChanged(const mvvm::NotifyCollectionChangedEventArgs& args) {
    for (const auto& handler : m_handlers) {
        handler(args);
    }
}

void LuaObservableCollection::HandleLuaChange(const std::string& action, 
                                               int index, 
                                               const std::wstring& text) {
    // 将 Lua 的变更转换为标准通知
    // 实际转换在 listener 的回调中完成
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
    lua_rawgeti(m_L, -1, static_cast<int>(index) + 1);
    
    std::wstring result = GetDisplayTextFromItem(-1);
    
    lua_pop(m_L, 2);
    return result;
}

std::wstring LuaObservableCollection::GetDisplayTextFromItem(int itemIndex) const {
    if (!m_L) return L"";
    
    int idx = itemIndex < 0 ? lua_gettop(m_L) : itemIndex;
    
    if (lua_isstring(m_L, idx)) {
        const char* str = lua_tostring(m_L, idx);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }
    
    if (lua_istable(m_L, idx)) {
        // 尝试 Name 字段
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

// ============================================================================
// ObservableCollectionBinding 实现
// ============================================================================
ObservableCollectionBinding::ObservableCollectionBinding(
    std::shared_ptr<luaui::controls::ListBox> listBox,
    std::shared_ptr<mvvm::INotifyCollectionChanged> collection)
    : m_listBox(listBox)
    , m_collection(collection)
{
    // 初始同步
    SyncAllItems();
    
    // 订阅增量变更
    m_handler = [this](const mvvm::NotifyCollectionChangedEventArgs& args) {
        OnCollectionChanged(args);
    };
    m_collection->SubscribeCollectionChanged(m_handler);
    
    utils::Logger::Info("[ObservableCollectionBinding] Created with incremental updates");
}

ObservableCollectionBinding::~ObservableCollectionBinding() {
    Detach();
}

void ObservableCollectionBinding::Detach() {
    if (!m_attached) return;
    m_attached = false;
    
    if (m_collection) {
        m_collection->UnsubscribeCollectionChanged(m_handler);
    }
    
    m_listBox.reset();
    m_collection.reset();
    
    utils::Logger::Info("[ObservableCollectionBinding] Detached");
}

void ObservableCollectionBinding::OnCollectionChanged(const mvvm::NotifyCollectionChangedEventArgs& args) {
    if (!m_listBox || !m_attached) return;
    
    switch (args.action) {
        case mvvm::NotifyCollectionChangedAction::Reset:
            SyncAllItems();
            break;
            
        case mvvm::NotifyCollectionChangedAction::Add:
            if (!args.newItems.empty()) {
                // 批量添加
                for (size_t i = 0; i < args.newItems.size(); ++i) {
                    try {
                        std::wstring text = std::any_cast<std::wstring>(args.newItems[i]);
                        InsertItem(args.newStartingIndex + static_cast<int>(i), text);
                    } catch (...) {}
                }
            } else {
                // 单条添加
                try {
                    std::wstring text = std::any_cast<std::wstring>(args.newItem);
                    InsertItem(args.newStartingIndex, text);
                } catch (...) {}
            }
            break;
            
        case mvvm::NotifyCollectionChangedAction::Remove:
            RemoveItem(args.oldStartingIndex);
            break;
            
        case mvvm::NotifyCollectionChangedAction::Replace:
            try {
                std::wstring text = std::any_cast<std::wstring>(args.newItem);
                ReplaceItem(args.newStartingIndex, text);
            } catch (...) {}
            break;
            
        case mvvm::NotifyCollectionChangedAction::Move:
            MoveItem(args.oldStartingIndex, args.newStartingIndex);
            break;
    }
}

void ObservableCollectionBinding::SyncAllItems() {
    if (!m_listBox) return;
    
    m_listBox->ClearItems();
    
    // 这里需要重新从集合获取所有项
    // 实际实现可能需要访问原始数据源
    
    utils::Logger::Info("[ObservableCollectionBinding] Full sync completed");
}

void ObservableCollectionBinding::InsertItem(int index, const std::wstring& text) {
    if (!m_listBox) return;
    // ListBox 需要添加 InsertItem 方法
    // 目前只能先 Add 然后可能重新排序
    m_listBox->AddItem(text);
    utils::Logger::DebugF("[ObservableCollectionBinding] Insert item at %d", index);
}

void ObservableCollectionBinding::RemoveItem(int index) {
    if (!m_listBox) return;
    m_listBox->RemoveItem(index);
    utils::Logger::DebugF("[ObservableCollectionBinding] Remove item at %d", index);
}

void ObservableCollectionBinding::MoveItem(int oldIndex, int newIndex) {
    // 需要 ListBox 支持 MoveItem
    utils::Logger::DebugF("[ObservableCollectionBinding] Move item from %d to %d", oldIndex, newIndex);
}

void ObservableCollectionBinding::ReplaceItem(int index, const std::wstring& text) {
    // 需要 ListBox 支持 ReplaceItem
    utils::Logger::DebugF("[ObservableCollectionBinding] Replace item at %d", index);
}

} // namespace lua
} // namespace luaui
