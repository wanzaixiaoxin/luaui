# Lua 绑定与脚本系统设计

## 1. 设计目标

- **安全性**: Lua脚本在沙箱中运行，限制危险操作
- **高效性**: 最小化Lua-C++交互开销
- **易用性**: 简洁直观的API设计
- **完整性**: 覆盖框架所有功能的绑定

## 2. Lua VM架构

```
┌─────────────────────────────────────────────────────────────────┐
│                     Lua VM 架构                                  │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │                    Lua Sandbox                             │ │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │ │
│  │  │ 受限IO   │  │ 受限OS   │  │ UI API   │  │ 框架API  │  │ │
│  │  │ (只读)   │  │ (安全)   │  │ (完整)   │  │ (完整)   │  │ │
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘  │ │
│  │                                                             │ │
│  │  限制: 内存上限 | 执行超时 | API白名单 | 文件沙盒          │ │
│  └───────────────────────────────────────────────────────────┘ │
│                            │                                    │
│                            ▼                                    │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │                    Binding Layer                           │ │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │ │
│  │  │ 自动绑定 │  │ 手动绑定 │  │ 属性绑定 │  │ 事件绑定 │  │ │
│  │  │ (sol2)   │  │ (lua_CFunction) │      │          │  │ │
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘  │ │
│  └───────────────────────────────────────────────────────────┘ │
│                            │                                    │
│                            ▼                                    │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │                    C++ Framework                           │ │
│  └───────────────────────────────────────────────────────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## 3. 核心类设计

```cpp
#pragma once
#include <lua.hpp>
#include <sol/sol.hpp>
#include <functional>
#include <memory>
#include <chrono>

namespace LuaUI {
namespace Lua {

// ==================== Lua沙箱 ====================

class LuaSandbox {
public:
    struct SecurityPolicy {
        size_t maxMemoryBytes = 64 * 1024 * 1024;  // 64MB
        std::chrono::milliseconds maxExecutionTime = std::chrono::seconds(30);
        bool allowFileWrite = false;
        bool allowNetwork = false;
        bool allowLoadLibrary = false;
        std::vector<std::string> allowedPaths;  // 允许访问的目录
    };
    
    LuaSandbox();
    ~LuaSandbox();
    
    // 初始化
    bool Initialize(const SecurityPolicy& policy = SecurityPolicy());
    void Shutdown();
    
    // 安全执行
    sol::protected_function_result Execute(const std::string& code, 
                                           const std::string& chunkName = "[string]");
    sol::protected_function_result ExecuteFile(const std::string& path);
    
    // 内存管理
    void SetMemoryLimit(size_t bytes);
    size_t GetMemoryUsage() const;
    void ForceGarbageCollect();
    
    // 执行时间限制
    void SetExecutionTimeout(std::chrono::milliseconds timeout);
    
    // 获取底层状态
    lua_State* GetLuaState() const;
    sol::state& GetSolState();
    
    // 注册API到Lua
    void RegisterUIAPI();
    void RegisterUtilityAPI();
    
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
    
    // 安全检查钩子
    static void MemoryCheckHook(lua_State* L, lua_Debug* ar);
    static void TimeoutHook(lua_State* L, lua_Debug* ar);
};

// ==================== 对象生命周期管理 ====================

// 用于在C++和Lua之间共享对象引用
template<typename T>
class LuaRef {
public:
    LuaRef() : m_ptr(nullptr), m_luaRef(LUA_NOREF) {}
    explicit LuaRef(T* ptr, lua_State* L);
    ~LuaRef();
    
    // 禁止拷贝，允许移动
    LuaRef(const LuaRef&) = delete;
    LuaRef& operator=(const LuaRef&) = delete;
    LuaRef(LuaRef&& other) noexcept;
    LuaRef& operator=(LuaRef&& other) noexcept;
    
    T* Get() const { return m_ptr; }
    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    
    bool IsValid() const { return m_ptr != nullptr; }
    void Reset();
    
private:
    T* m_ptr;
    lua_State* m_luaState;
    int m_luaRef;
};

// ==================== UI元素绑定 ====================

class UIElementBinder {
public:
    static void Bind(sol::state& lua);
    
private:
    // 注册UIElement基类
    static void RegisterUIElement(sol::state& lua);
    static void RegisterFrameworkElement(sol::state& lua);
    static void RegisterControl(sol::state& lua);
    
    // 注册具体控件
    static void RegisterWindow(sol::state& lua);
    static void RegisterButton(sol::state& lua);
    static void RegisterTextBox(sol::state& lua);
    static void RegisterLabel(sol::state& lua);
    static void RegisterPanel(sol::state& lua);
    static void RegisterGrid(sol::state& lua);
    static void RegisterStackPanel(sol::state& lua);
    static void RegisterImage(sol::state& lua);
    static void RegisterListBox(sol::state& lua);
    static void RegisterTreeView(sol::state& lua);
    static void RegisterProgressBar(sol::state& lua);
    static void RegisterSlider(sol::state& lua);
    static void RegisterCheckBox(sol::state& lua);
    static void RegisterRadioButton(sol::state& lua);
    static void RegisterComboBox(sol::state& lua);
    static void RegisterMenu(sol::state& lua);
    static void RegisterToolBar(sol::state& lua);
    static void RegisterStatusBar(sol::state& lua);
    static void RegisterTabControl(sol::state& lua);
    static void RegisterDataGrid(sol::state& lua);
};

// ==================== 属性系统绑定 ====================

class PropertyBinder {
public:
    static void Bind(sol::state& lua);
    
    // 创建依赖属性
    template<typename T>
    static void RegisterDependencyProperty(const std::string& name);
    
    // 属性变更通知
    static void NotifyPropertyChanged(sol::object viewModel, const std::string& propertyName);
};

// ==================== 命令系统绑定 ====================

class CommandBinder {
public:
    static void Bind(sol::state& lua);
    
    // 创建命令
    static sol::object CreateCommand(sol::function execute, sol::optional<sol::function> canExecute);
    
    // 刷新命令状态
    static void InvalidateCommand(sol::object command);
};

// ==================== 数据绑定绑定 ====================

class BindingBinder {
public:
    static void Bind(sol::state& lua);
    
    // 创建绑定
    static sol::object CreateBinding(const std::string& path, 
                                     sol::optional<std::string> mode,
                                     sol::optional<sol::object> converter);
    
    // 设置绑定
    static void SetBinding(sol::object element, const std::string& property, sol::object binding);
};

// ==================== 事件系统绑定 ====================

class EventBinder {
public:
    static void Bind(sol::state& lua);
    
    // 订阅事件
    static sol::object Subscribe(sol::object element, const std::string& eventName, 
                                  sol::function handler);
    
    // 取消订阅
    static void Unsubscribe(sol::object subscription);
    
    // 触发事件（用于测试）
    static void RaiseEvent(sol::object element, const std::string& eventName, sol::object args);
};

// ==================== 动画系统绑定 ====================

class AnimationBinder {
public:
    static void Bind(sol::state& lua);
    
    // 属性动画
    static sol::object CreatePropertyAnimation(sol::object target, const std::string& property);
    static void StartAnimation(sol::object animation);
    static void StopAnimation(sol::object animation);
    static void PauseAnimation(sol::object animation);
    
    // 故事板
    static sol::object CreateStoryboard();
    static void AddAnimationToStoryboard(sol::object storyboard, sol::object animation, 
                                          sol::optional<int> delay);
    static void BeginStoryboard(sol::object storyboard);
    
    // 缓动函数
    static void RegisterEasingFunctions(sol::state& lua);
};

// ==================== 资源系统绑定 ====================

class ResourceBinder {
public:
    static void Bind(sol::state& lua);
    
    // 加载资源
    static sol::object LoadImage(const std::string& path);
    static sol::object LoadString(const std::string& key);
    static sol::object LoadStyle(const std::string& key);
    
    // 获取主题资源
    static sol::object GetThemeResource(const std::string& key);
};

// ==================== 对话框绑定 ====================

class DialogBinder {
public:
    static void Bind(sol::state& lua);
    
    // 消息框
    static void ShowMessage(const std::string& message, sol::optional<std::string> title,
                            sol::optional<std::string> icon);
    static bool ShowConfirm(const std::string& message, sol::optional<std::string> title);
    static sol::object ShowInput(const std::string& message, sol::optional<std::string> defaultValue);
    
    // 文件对话框
    static sol::object OpenFileDialog(sol::table options);
    static sol::object SaveFileDialog(sol::table options);
    static sol::object SelectFolderDialog(sol::optional<std::string> title);
};

// ==================== 网络请求绑定 ====================

class HttpBinder {
public:
    static void Bind(sol::state& lua);
    
    // HTTP方法
    static void Get(const std::string& url, sol::table options);
    static void Post(const std::string& url, sol::table options);
    static void Put(const std::string& url, sol::table options);
    static void Delete(const std::string& url, sol::table options);
    
    // WebSocket
    static sol::object CreateWebSocket(const std::string& url);
};

// ==================== 存储绑定 ====================

class StorageBinder {
public:
    static void Bind(sol::state& lua);
    
    // 设置存储
    static sol::object GetSettings();
    static void SetSetting(const std::string& key, sol::object value);
    static sol::object GetSetting(const std::string& key, sol::optional<sol::object> defaultValue);
    static void SaveSettings();
    
    // 本地数据库
    static sol::object OpenDatabase(const std::string& path);
};

// ==================== 多线程绑定 ====================

class TaskBinder {
public:
    static void Bind(sol::state& lua);
    
    // 异步任务
    static sol::object Run(sol::function task);
    static sol::object RunWithProgress(sol::function task);
    static sol::object Delay(int milliseconds);
    
    // 线程同步
    static void InvokeOnUI(sol::function action);
    static bool CheckAccess();  // 检查是否在UI线程
};

// ==================== 日志绑定 ====================

class LoggerBinder {
public:
    static void Bind(sol::state& lua);
    
    static void Debug(const std::string& message);
    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);
    static void Fatal(const std::string& message);
    
    // 格式化日志
    static void DebugF(const std::string& format, sol::table args);
    static void InfoF(const std::string& format, sol::table args);
};

// ==================== 调试绑定 ====================

class DebugBinder {
public:
    static void Bind(sol::state& lua);
    
    static void Inspect(sol::object obj);
    static sol::table Profile(sol::function fn);
    static void DumpMemory(const std::string& filename);
    static void VisualizeTree();
    static void Break();
};

// ==================== 全局UI对象 ====================

class UIGlobal {
public:
    static void Bind(sol::state& lua);
    
    // 控件查找
    static sol::object GetControl(const std::string& id);
    static sol::table FindControls(const std::string& selector);
    static sol::object GetFocusedControl();
    
    // 主题
    static void SetTheme(const std::string& themeName);
    static std::string GetCurrentTheme();
    
    // 全局事件
    static void RegisterGlobalEvent(const std::string& name, sol::function handler);
    
    // 退出应用
    static void Exit(sol::optional<int> exitCode);
};

// ==================== 视图模型基类 ====================

// 用于Lua中创建ViewModel的基类
class ViewModelBase {
public:
    virtual ~ViewModelBase() = default;
    
    // 属性变更通知
    void NotifyPropertyChanged(const std::string& propertyName);
    void NotifyPropertiesChanged(const std::vector<std::string>& propertyNames);
    
    // 批量更新
    void BeginUpdate();
    void EndUpdate();
    
    // 生命周期
    virtual void OnCreated();
    virtual void OnActivated();
    virtual void OnDeactivated();
    virtual void OnDestroy();
    
    // Lua对象引用
    void SetLuaObject(sol::object obj);
    sol::object GetLuaObject() const;
    
private:
    sol::object m_luaObject;
    int m_updateDepth = 0;
    std::vector<std::string> m_pendingNotifications;
};

// ==================== 脚本引擎 ====================

class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();
    
    bool Initialize();
    void Shutdown();
    
    // 加载脚本
    bool LoadScript(const std::string& path);
    bool LoadViewModel(const std::string& path, const std::string& typeName);
    
    // 执行函数
    sol::protected_function_result CallFunction(const std::string& name, sol::variadic_args args);
    
    // 获取全局变量
    sol::object GetGlobal(const std::string& name);
    void SetGlobal(const std::string& name, sol::object value);
    
    // 创建实例
    sol::object CreateInstance(const std::string& className, sol::variadic_args args);
    
    // 热重载
    bool ReloadScript(const std::string& path);
    
private:
    std::unique_ptr<LuaSandbox> m_sandbox;
    std::unordered_map<std::string, std::string> m_loadedScripts;
};

}} // namespace LuaUI::Lua
```

## 4. Lua API 详细设计

### 4.1 UI 命名空间

```lua
-- UI 命名空间提供所有UI相关功能
local UI = require("UI")

-- ==================== 控件操作 ====================

-- 获取控件
local button = UI.getControl("myButton")
local controls = UI.findControls(".button-class")  -- 按样式类
local controls = UI.findControls("#container .item")  -- CSS选择器风格

-- 控件属性
button:setProperty("text", "Click Me")
button:setProperty("width", 120)
local text = button:getProperty("text")

-- 批量设置
button:setProperties({
    text = "New Text",
    width = 120,
    height = 40,
    background = "#FF5722"
})

-- 数据绑定
button:setBinding("text", "ViewModel.ButtonText", UI.Binding.Mode.OneWay)
button:setBinding("isEnabled", "ViewModel.CanClick")

-- ==================== 事件处理 ====================

-- 简单事件绑定
button:onClick(function(sender, args)
    print("Clicked!")
end)

-- 一次性事件
button:once("click", function() end)

-- 命令绑定（MVVM）
button:setCommand("ViewModel.SubmitCommand")
button:setCommandParameter({ id = 123 })

-- 多事件订阅
local sub = button:subscribe({
    onClick = function(sender, args) end,
    onMouseEnter = function(sender, args) end,
    onMouseLeave = function(sender, args) end,
    onPropertyChanged = function(sender, args)
        if args.property == "text" then
            print("Text changed to: " .. args.newValue)
        end
    end
})

-- 取消订阅
sub:unsubscribe()

-- ==================== 动画 ====================

-- 简单属性动画
button:animate({
    property = "opacity",
    from = 1.0,
    to = 0.5,
    duration = 300,  -- ms
    easing = UI.Animation.Easing.EaseInOutQuad
})

-- 多属性动画
button:animate({
    properties = {
        { property = "opacity", from = 1, to = 0 },
        { property = "scaleX", from = 1, to = 1.1 },
        { property = "scaleY", from = 1, to = 1.1 },
    },
    duration = 200
})

-- 故事板
local storyboard = UI.Animation.Storyboard()
storyboard:add(UI.Animation.PropertyAnimation(button, "width", {
    to = 200,
    duration = 500,
    easing = UI.Animation.Easing.Bounce
}))
storyboard:add(UI.Animation.PropertyAnimation(button, "background", {
    to = "#4CAF50",
    duration = 300
}), 200)  -- 延迟200ms
storyboard:start()

-- 动画完成回调
storyboard:onCompleted(function()
    print("Animation done!")
end)

-- ==================== 对话框 ====================

-- 简单消息
UI.showMessage("操作成功", UI.MessageIcon.Info)

-- 确认对话框
UI.showConfirm("确定要删除吗？", function(result)
    if result == UI.DialogResult.Yes then
        -- 执行删除
    end
end, {
    title = "确认删除",
    icon = UI.MessageIcon.Warning
})

-- 输入对话框
UI.showInput("请输入用户名:", function(result)
    if result.confirmed then
        print("输入: " .. result.value)
    end
end, {
    defaultValue = "guest",
    title = "登录"
})

-- 文件对话框
UI.openFileDialog({
    title = "选择图片",
    filter = "图片文件|*.png;*.jpg;*.jpeg|所有文件|*.*",
    multiSelect = false,
    onSelected = function(files)
        if #files > 0 then
            print("选择了: " .. files[1])
        end
    end
})

-- 保存对话框
UI.saveFileDialog({
    title = "保存文件",
    filter = "文本文件|*.txt",
    defaultExt = "txt",
    onSelected = function(file)
        -- 保存文件
    end
})

-- ==================== 主题 ====================

-- 设置主题
UI.setTheme("dark")

-- 获取主题颜色
local bgColor = UI.getThemeResource("BackgroundBrush")
local primaryColor = UI.getThemeResource("PrimaryColor")

-- 监听主题变化
UI.onThemeChanged(function(newTheme)
    print("Theme changed to: " .. newTheme)
end)

-- ==================== 全局方法 ====================

-- 退出应用
UI.exit(0)

-- 获取主窗口
local mainWindow = UI.getMainWindow()

-- 设置状态栏文本
UI.setStatusBarText("Ready", UI.StatusBarPanel.Main)

-- 显示通知
UI.showNotification({
    title = "下载完成",
    message = "文件已保存到下载文件夹",
    type = UI.NotificationType.Success
})
```

### 4.2 ViewModel 基类

```lua
-- ViewModel基类
local ViewModel = require("UI.ViewModel")

-- 定义ViewModel
local MyViewModel = ViewModel:extend("MyViewModel")

-- 定义属性（自动生成getter/setter和变更通知）
MyViewModel:defineProperties({
    UserName = { 
        type = "string", 
        default = "",
        validate = function(value)
            return #value >= 3, "用户名至少3个字符"
        end
    },
    Password = { type = "string", default = "" },
    IsLoggedIn = { type = "boolean", default = false },
    UserList = { type = "ObservableCollection", itemType = "User" },
})

-- 定义命令（自动生成CanExecute检查）
MyViewModel:defineCommands({
    Login = {
        execute = function(self)
            self.IsLoggedIn = true
            self:addMessage("登录成功")
        end,
        canExecute = function(self)
            return #self.UserName >= 3 and #self.Password >= 6
        end
    },
    Logout = function(self)
        self.IsLoggedIn = false
        self.UserName = ""
        self.Password = ""
    end
})

-- 生命周期回调
function MyViewModel:onCreated()
    -- 初始化
end

function MyViewModel:onActivated()
    -- 页面激活时调用
    self:loadData()
end

function MyViewModel:onDeactivated()
    -- 页面失活时调用
end

function MyViewModel:onDestroy()
    -- 清理资源
end

-- 辅助方法
function MyViewModel:loadData()
    -- 异步加载数据
    UI.Task.run(function()
        -- 后台线程
        return fetchDataFromServer()
    end):continueOnUI(function(data)
        -- 回到UI线程
        for _, user in ipairs(data) do
            self.UserList:add(user)
        end
    end)
end

function MyViewModel:addMessage(text)
    -- 批量更新优化
    self:beginUpdate()
    self.MessageText = text
    self.MessageVisible = true
    self:endUpdate()
end

return MyViewModel
```

### 4.3 存储 API

```lua
local Storage = require("UI.Storage")

-- ==================== 应用设置 ====================

local settings = Storage.Settings.get()

-- 读取设置
local username = settings:get("username", "default")
local windowSize = settings:get("window.size", { width = 800, height = 600 })

-- 写入设置
settings:set("username", "John")
settings:set("window.size", { width = 1024, height = 768 })
settings:save()

-- 监听设置变更
settings:onChanged(function(key, oldValue, newValue)
    print("Setting " .. key .. " changed")
end)

-- ==================== 本地数据库 ====================

local db = Storage.Database.open("app_data.db")

-- 执行SQL
db:execute([[
    CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY,
        name TEXT NOT NULL,
        email TEXT
    )
]])

-- 插入数据
db:execute("INSERT INTO users (name, email) VALUES (?, ?)", { "Alice", "alice@example.com" })

-- 查询数据
local users = db:query("SELECT * FROM users WHERE name = ?", { "Alice" })
for _, user in ipairs(users) do
    print(user.id, user.name, user.email)
end

-- 事务
db:transaction(function()
    db:execute("INSERT INTO users (name) VALUES (?)", { "Bob" })
    db:execute("INSERT INTO users (name) VALUES (?)", { "Charlie" })
end)

-- 关闭连接
db:close()
```

### 4.4 HTTP API

```lua
local Http = require("UI.Http")

-- ==================== GET请求 ====================

Http.get("https://api.example.com/users", {
    headers = {
        ["Authorization"] = "Bearer " .. token,
        ["Accept"] = "application/json"
    },
    timeout = 5000,
    onSuccess = function(response)
        local data = response:json()
        print("Got " .. #data .. " users")
    end,
    onError = function(error)
        print("Error: " .. error.message)
    end,
    onProgress = function(loaded, total)
        local percent = loaded / total * 100
        print("Progress: " .. percent .. "%")
    end
})

-- ==================== POST请求 ====================

Http.post("https://api.example.com/users", {
    headers = {
        ["Content-Type"] = "application/json"
    },
    body = {
        name = "John",
        email = "john@example.com"
    },  -- 自动序列化为JSON
    onSuccess = function(response)
        print("Created user with ID: " .. response:json().id)
    end
})

-- Form data
Http.post("https://api.example.com/upload", {
    contentType = "multipart/form-data",
    body = {
        file = Http.File("path/to/file.png"),
        description = "My upload"
    },
    onSuccess = function(response)
        print("Upload complete")
    end
})

-- ==================== WebSocket ====================

local ws = Http.WebSocket("wss://echo.websocket.org")

ws:onOpen(function()
    print("WebSocket connected")
    ws:send("Hello Server!")
end)

ws:onMessage(function(message)
    print("Received: " .. message)
end)

ws:onError(function(error)
    print("WebSocket error: " .. error)
end)

ws:onClose(function(code, reason)
    print("WebSocket closed: " .. reason)
end)

-- 关闭连接
ws:close()
```

### 4.5 多线程 API

```lua
local Task = require("UI.Task")

-- ==================== 异步任务 ====================

-- 简单异步任务
Task.run(function()
    -- 在后台线程执行
    local result = performHeavyCalculation()
    return result
end):continueOnUI(function(result)
    -- 回到UI线程更新界面
    ViewModel.Result = result
end)

-- 带取消令牌的任务
local cts = Task.CancellationTokenSource()
Task.run(function(token)
    for i = 1, 100 do
        if token:isCancellationRequested() then
            return nil
        end
        Task.delay(100)  -- 模拟工作
        token:reportProgress(i)
    end
    return "Done"
end, {
    cancellationToken = cts.token,
    onProgress = function(value)
        ViewModel.Progress = value
    end,
    onComplete = function(result)
        if result then
            print("Completed: " .. result)
        else
            print("Cancelled")
        end
    end
})

-- 取消任务
cts:cancel()

-- ==================== 延迟执行 ====================

Task.delay(1000):continueWith(function()
    print("1秒后执行")
end)

-- ==================== UI线程操作 ====================

-- 检查当前线程
if not Task.isUIThread() then
    Task.invokeOnUI(function()
        -- 在UI线程执行
        ViewModel.Status = "Updated"
    end)
end

-- ==================== 并行任务 ====================

local tasks = {
    Task.run(function() return fetchData1() end),
    Task.run(function() return fetchData2() end),
    Task.run(function() return fetchData3() end),
}

Task.whenAll(tasks):continueOnUI(function(results)
    ViewModel.Data1 = results[1]
    ViewModel.Data2 = results[2]
    ViewModel.Data3 = results[3]
end)

-- 任一完成
Task.whenAny(tasks):continueWith(function(completedTask)
    print("First task completed")
end)
```

## 5. 绑定实现示例

```cpp
// 使用 sol2 实现自动绑定
void UIElementBinder::RegisterButton(sol::state& lua) {
    lua.new_usertype<UI::Button>("Button",
        sol::base_classes, sol::bases<UI::Control, UI::FrameworkElement, UI::UIElement>(),
        
        // 属性
        "content", sol::property(
            &UI::Button::GetContent,
            &UI::Button::SetContent
        ),
        "isDefault", sol::property(
            &UI::Button::IsDefault,
            &UI::Button::SetIsDefault
        ),
        "isCancel", sol::property(
            &UI::Button::IsCancel,
            &UI::Button::SetIsCancel
        ),
        
        // 方法
        "onClick", [](UI::Button& self, sol::function handler) {
            return self.Click.Subscribe([handler](auto& sender, auto& args) {
                handler(&sender, &args);
            });
        },
        "click", &UI::Button::PerformClick,
        
        // 命令
        "setCommand", &UI::Button::SetCommand,
        "setCommandParameter", &UI::Button::SetCommandParameter
    );
}

// 属性变更通知绑定
void PropertyBinder::NotifyPropertyChanged(sol::object viewModel, const std::string& propertyName) {
    if (viewModel.is<UI::Lua::ViewModelBase*>()) {
        auto* vm = viewModel.as<UI::Lua::ViewModelBase*>();
        vm->NotifyPropertyChanged(propertyName);
    }
}

// 安全执行封装
sol::protected_function_result LuaSandbox::Execute(const std::string& code, 
                                                    const std::string& chunkName) {
    // 设置执行超时
    auto startTime = std::chrono::steady_clock::now();
    lua_sethook(m_impl->L, TimeoutHook, LUA_MASKCOUNT, 1000);
    
    // 执行代码
    auto result = m_impl->lua.safe_script(code, sol::script_pass_on_error, chunkName);
    
    // 检查超时
    if (std::chrono::steady_clock::now() - startTime > m_impl->policy.maxExecutionTime) {
        return sol::protected_function_result(
            m_impl->L, -1, 0, 0, sol::call_status::runtime
        );
    }
    
    return result;
}
```

## 6. 安全策略实现

```cpp
void LuaSandbox::SetupSecurity(const SecurityPolicy& policy) {
    lua_State* L = m_impl->L;
    
    // 1. 移除危险函数
    lua_pushnil(L);
    lua_setglobal(L, "dofile");
    lua_pushnil(L);
    lua_setglobal(L, "loadfile");
    lua_pushnil(L);
    lua_setglobal(L, "load");
    
    // 2. 限制IO库
    lua_getglobal(L, "io");
    if (lua_istable(L, -1)) {
        // 只保留只读操作
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            const char* name = lua_tostring(L, -2);
            if (strcmp(name, "open") == 0 || 
                strcmp(name, "lines") == 0 ||
                strcmp(name, "read") == 0) {
                // 保留这些函数
            } else {
                lua_pushnil(L);
                lua_setfield(L, -4, name);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    // 3. 限制OS库
    lua_getglobal(L, "os");
    if (lua_istable(L, -1)) {
        // 只保留时间相关函数
        std::vector<const char*> allowed = {"clock", "date", "difftime", "time"};
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            const char* name = lua_tostring(L, -2);
            bool isAllowed = false;
            for (auto* a : allowed) {
                if (strcmp(name, a) == 0) {
                    isAllowed = true;
                    break;
                }
            }
            if (!isAllowed) {
                lua_pushnil(L);
                lua_setfield(L, -4, name);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    // 4. 设置内存限制
    lua_gc(L, LUA_GCSETPAUSE, 100);
    lua_gc(L, LUA_GCSETSTEPMUL, 400);
    
    // 5. 禁用debug库的某些功能
    lua_getglobal(L, "debug");
    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        lua_setfield(L, -2, "setupvalue");
        lua_pushnil(L);
        lua_setfield(L, -2, "upvaluejoin");
        lua_pushnil(L);
        lua_setfield(L, -2, "getregistry");
    }
    lua_pop(L, 1);
    
    // 6. 替换require为安全版本
    lua_pushcfunction(L, SafeRequire);
    lua_setglobal(L, "require");
}
```
