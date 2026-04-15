// Lua MVVM Binding - 支持 Lua 端触发属性变更通知

#include "LuaBinding_MVVM.h"
#include "LuaSandbox.h"
#include "Logger.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

// ============================================================================
// AutoViewModel Framework - Lua 源代码
// 在 C++ 层注册，让所有 Lua 脚本都能使用
// ============================================================================
static const char* AUTO_VIEWMODEL_FRAMEWORK = R"LUAFW(
-- LuaUI AutoViewModel Framework (Auto-registered by C++)
-- 提供自动属性通知、批量更新、计算属性等功能

local AutoViewModel = {}
-- __index 设为函数，优先从 _data 表中查找动态属性
AutoViewModel.__index = function(t, k)
    local v = rawget(t._data, k)
    if v ~= nil then return v end
    return AutoViewModel[k]
end

function AutoViewModel.new()
    local self = setmetatable({}, AutoViewModel)
    self._data = {}
    self._batchMode = false
    self._pendingNotifications = {}
    self._computed = {}
    return self
end

function AutoViewModel:EnableAutoNotify()
    local selfRef = self
    local proxy = {}
    local mt = {
        __index = function(t, k)
            -- 特殊处理 _data 访问，返回原始表的 _data（供 C++ 层直接访问）
            if k == "_data" then
                return selfRef._data
            end
            local v = rawget(selfRef._data, k)
            if v ~= nil then return v end
            v = rawget(selfRef, k)
            if v ~= nil then return v end
            return AutoViewModel[k]
        end,
        __newindex = function(t, k, v)
            local oldValue = rawget(selfRef._data, k)
            if oldValue ~= v then
                -- Log.debug("[__newindex] Setting '" .. tostring(k) .. "': " .. tostring(oldValue) .. " -> " .. tostring(v))
                rawset(selfRef._data, k, v)
                -- Log.debug("[__newindex] Notifying computed change for '" .. tostring(k) .. "'")
                selfRef:_notifyComputedChange(k)
                if selfRef._batchMode then
                    selfRef._pendingNotifications[k] = true
                else
                    selfRef:_notifyChange(k)
                end
            else
                -- Log.debug("[__newindex] Value unchanged for '" .. tostring(k) .. "': " .. tostring(v))
            end
        end
    }
    setmetatable(proxy, mt)
    return proxy
end

function AutoViewModel:_notifyChange(propertyName)
    if _G.Notify and type(_G.Notify) == "function" then
        _G.Notify(propertyName)
    end
end

function AutoViewModel:_notifyComputedChange(changedProperty)
    -- Log.debug("[_notifyComputedChange] Property changed: " .. tostring(changedProperty))
    for name, computed in pairs(self._computed) do
        for _, dep in ipairs(computed.deps) do
            if dep == changedProperty then
                local newValue = computed.fn(self)
                -- Log.debug("[_notifyComputedChange] Recalculating '" .. name .. "': " .. tostring(newValue))
                rawset(self._data, name, newValue)
                if self._batchMode then
                    self._pendingNotifications[name] = true
                else
                    self:_notifyChange(name)
                end
                break
            end
        end
    end
end

function AutoViewModel:BeginBatch()
    self._batchMode = true
    self._pendingNotifications = {}
end

function AutoViewModel:EndBatch()
    self._batchMode = false
    for propName, _ in pairs(self._pendingNotifications) do
        self:_notifyChange(propName)
    end
    self._pendingNotifications = {}
end

function AutoViewModel:DefineComputed(name, deps, computeFn)
    self._computed[name] = { deps = deps, fn = computeFn }
    local initialValue = computeFn(self)
    rawset(self._data, name, initialValue)
end

function AutoViewModel:GetRaw(propertyName)
    return rawget(self._data, propertyName)
end

function AutoViewModel:SetRaw(propertyName, value)
    rawset(self._data, propertyName, value)
end

-- ============================================================================
-- 数组操作方法 - 支持集合绑定
-- ============================================================================

-- 添加项到数组并触发通知
function AutoViewModel:AddItem(arrayPropertyName, item)
    local arr = rawget(self._data, arrayPropertyName)
    if type(arr) ~= "table" then
        arr = {}
        rawset(self._data, arrayPropertyName, arr)
    end
    table.insert(arr, item)
    
    -- 触发属性变更通知
    if self._batchMode then
        self._pendingNotifications[arrayPropertyName] = true
    else
        self:_notifyChange(arrayPropertyName)
    end
    
    return #arr
end

-- 移除数组中的项
function AutoViewModel:RemoveItem(arrayPropertyName, index)
    local arr = rawget(self._data, arrayPropertyName)
    if type(arr) ~= "table" then return nil end
    
    if index >= 1 and index <= #arr then
        local item = table.remove(arr, index)
        
        -- 触发属性变更通知
        if self._batchMode then
            self._pendingNotifications[arrayPropertyName] = true
        else
            self:_notifyChange(arrayPropertyName)
        end
        
        return item
    end
    return nil
end

-- 清空数组
function AutoViewModel:ClearItems(arrayPropertyName)
    local arr = rawget(self._data, arrayPropertyName)
    if type(arr) ~= "table" then return end
    
    -- 清空表
    for k in pairs(arr) do
        arr[k] = nil
    end
    
    -- 触发属性变更通知
    if self._batchMode then
        self._pendingNotifications[arrayPropertyName] = true
    else
        self:_notifyChange(arrayPropertyName)
    end
end

-- ObservableCollection - 支持增量更新的集合
local ObservableCollection = {}
ObservableCollection.__index = ObservableCollection

function ObservableCollection.new()
    local self = setmetatable({}, ObservableCollection)
    self._items = {}
    self._listeners = {}
    self._nextToken = 1
    return self
end

-- 订阅集合变更（供 C++ 端调用）
function ObservableCollection:Subscribe(callback)
    local token = self._nextToken
    self._nextToken = self._nextToken + 1
    self._listeners[token] = callback
    
    Log.debug("[ObservableCollection] Subscribed, token=" .. token)
    
    -- 返回订阅 token，用于取消订阅
    return token
end

-- 取消订阅
function ObservableCollection:Unsubscribe(token)
    if self._listeners[token] then
        self._listeners[token] = nil
        Log.debug("[ObservableCollection] Unsubscribed, token=" .. token)
    end
end

-- 内部通知方法
function ObservableCollection:_notify(action, item, index)
    Log.debug("[ObservableCollection] Notify: " .. action .. " at " .. index)
    for token, callback in pairs(self._listeners) do
        local success, err = pcall(callback, action, index, item)
        if not success then
            Log.error("[ObservableCollection] Listener error: " .. tostring(err))
        end
    end
end

function ObservableCollection:Add(item)
    table.insert(self._items, item)
    self:_notify("Add", item, #self._items)
end

function ObservableCollection:Insert(index, item)
    if index < 1 or index > #self._items + 1 then return end
    table.insert(self._items, index, item)
    self:_notify("Add", item, index)
end

function ObservableCollection:Remove(item)
    for i, v in ipairs(self._items) do
        if v == item then
            table.remove(self._items, i)
            self:_notify("Remove", item, i)
            return true
        end
    end
    return false
end

function ObservableCollection:RemoveAt(index)
    if index < 1 or index > #self._items then return nil end
    local item = table.remove(self._items, index)
    self:_notify("Remove", item, index)
    return item
end

function ObservableCollection:Clear()
    self._items = {}
    self:_notify("Reset", nil, 0)
end

function ObservableCollection:Replace(index, newItem)
    if index < 1 or index > #self._items then return end
    local oldItem = self._items[index]
    self._items[index] = newItem
    self:_notify("Replace", newItem, index)
    return oldItem
end

function ObservableCollection:Move(oldIndex, newIndex)
    if oldIndex < 1 or oldIndex > #self._items then return end
    if newIndex < 1 or newIndex > #self._items then return end
    
    local item = table.remove(self._items, oldIndex)
    table.insert(self._items, newIndex, item)
    self:_notify("Move", item, newIndex)  -- C++ 端需要根据 action 处理
end

function ObservableCollection:Get(index)
    return self._items[index]
end

function ObservableCollection:Count()
    return #self._items
end

function ObservableCollection:GetAll()
    -- 返回副本
    local copy = {}
    for i, v in ipairs(self._items) do
        copy[i] = v
    end
    return copy
end

-- 兼容旧版的 Remove 方法
function ObservableCollection:_oldRemove(item)
    for i, v in ipairs(self._items) do
        if v == item then
            table.remove(self._items, i)
            self:_notify("Remove", item, i)
            return true
        end
    end
    return false
end

function ObservableCollection:RemoveAt(index)
    if index >= 1 and index <= #self._items then
        local item = self._items[index]
        table.remove(self._items, index)
        self:_notify("Removed", item, index)
        return item
    end
    return nil
end

function ObservableCollection:Clear()
    self._items = {}
    self:_notify("Cleared", nil, 0)
end

function ObservableCollection:Get(index)
    return self._items[index]
end

function ObservableCollection:Count()
    return #self._items
end

function ObservableCollection:GetAll()
    return self._items
end

function ObservableCollection:Subscribe(callback)
    table.insert(self._listeners, callback)
end

function ObservableCollection:_notify(action, item, index)
    for _, callback in ipairs(self._listeners) do
        callback(action, item, index)
    end
end

-- Command Pattern
local Command = {}
Command.__index = Command

function Command.new(execute, canExecute)
    local self = setmetatable({}, Command)
    self._execute = execute or function() end
    self._canExecute = canExecute or function() return true end
    return self
end

function Command:Execute(parameter)
    if self:CanExecute(parameter) then
        self._execute(parameter)
    end
end

function Command:CanExecute(parameter)
    return self._canExecute(parameter)
end

-- Export to global
_G.AutoViewModel = AutoViewModel
_G.ObservableCollection = ObservableCollection
_G.Command = Command

Log.info("[Framework] AutoViewModel Framework loaded (from C++)")
)LUAFW";

namespace luaui {
namespace lua {

// Global registry for property notifiers (LuaState -> Notifier)
// Using shared_ptr to ensure notifier stays alive as long as Lua state is active
static std::unordered_map<lua_State*, std::shared_ptr<LuaPropertyNotifier>> g_notifiers;

void RegisterPropertyNotifier(lua_State* L, std::shared_ptr<LuaPropertyNotifier> notifier) {
    g_notifiers[L] = notifier;
}

void UnregisterPropertyNotifier(lua_State* L) {
    g_notifiers.erase(L);
}

// ============================================================================
// MVVM Binding Registration
// ============================================================================

void LuaBinding::RegisterMVVM(lua_State* L) {
    // 创建 Notify 全局函数
    // 用法: Notify("PropertyName") 或 Notify(ViewModel, "PropertyName")
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* propertyName = nullptr;
        
        if (lua_gettop(L) >= 2 && lua_isstring(L, 2)) {
            // Notify(ViewModel, "PropertyName")
            propertyName = lua_tostring(L, 2);
        } else if (lua_gettop(L) >= 1 && lua_isstring(L, 1)) {
            // Notify("PropertyName") - 使用注册的通知器
            propertyName = lua_tostring(L, 1);
        }
        
        if (!propertyName) {
            luaL_error(L, "Notify expects a property name string");
            return 0;
        }
        
        // 查找已注册的 notifier
        auto it = g_notifiers.find(L);
        if (it != g_notifiers.end()) {
            auto notifier = it->second;  // shared_ptr, no .lock() needed
            if (notifier) {
                notifier->NotifyPropertyChanged(propertyName);
                utils::Logger::DebugF("[Lua MVVM] Property changed: %s", propertyName);
            }
        }
        
        return 0;
    });
    lua_setglobal(L, "Notify");
    
    // 创建 ViewModelBase 元表，提供 Notify 方法
    luaL_newmetatable(L, "LuaUI.ViewModelBase");
    
    // __index 表
    lua_newtable(L);
    
    // ViewModelBase:Notify(propertyName)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        luaL_checktype(L, 1, LUA_TTABLE);  // self
        const char* propertyName = luaL_checkstring(L, 2);
        
        auto it = g_notifiers.find(L);
        if (it != g_notifiers.end()) {
            auto notifier = it->second;  // shared_ptr, no .lock() needed
            if (notifier) {
                notifier->NotifyPropertyChanged(propertyName);
            }
        }
        
        return 0;
    });
    lua_setfield(L, -2, "Notify");
    
    // 设置 __index
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
    
    // 注册 AutoViewModel Framework 到 Lua 全局
    int result = luaL_loadstring(L, AUTO_VIEWMODEL_FRAMEWORK);
    if (result == LUA_OK) {
        result = lua_pcall(L, 0, 0, 0);
        if (result == LUA_OK) {
            utils::Logger::Info("[Lua MVVM] AutoViewModel Framework registered");
        } else {
            const char* error = lua_tostring(L, -1);
            utils::Logger::ErrorF("[Lua MVVM] Failed to register Framework: %s", error ? error : "unknown");
            lua_pop(L, 1);
        }
    } else {
        const char* error = lua_tostring(L, -1);
        utils::Logger::ErrorF("[Lua MVVM] Failed to load Framework: %s", error ? error : "unknown");
        lua_pop(L, 1);
    }
    
    utils::Logger::Info("[Lua MVVM] Binding registered");
}

} // namespace lua
} // namespace luaui
