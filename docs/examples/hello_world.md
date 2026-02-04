# Hello World 示例

## 概述

这是一个简单的Hello World示例，展示了LuaUI框架的基本用法。

## 文件结构

```
examples/hello_world/
├── main.cpp      # C++主程序
├── main.lua      # Lua脚本
└── layout.xml    # XML布局文件
```

## 代码说明

### 1. XML布局 (layout.xml)

定义了一个简单的窗口，包含两个按钮和一个标签：

```xml
<window id="mainWindow" title="Hello World - LuaUI" width="400" height="300">
    <label id="lblTitle" text="Welcome to LuaUI!" x="100" y="30" width="200" height="30"/>
    <button id="btnHello" text="Click Me" x="150" y="100" width="100" height="35"/>
    <label id="lblMessage" text="" x="100" y="150" width="200" height="25"/>
    <button id="btnExit" text="Exit" x="150" y="200" width="100" height="35"/>
</window>
```

### 2. Lua脚本 (main.lua)

处理按钮点击事件：

```lua
-- 按钮点击事件处理
function onHelloClick()
    clickCount = clickCount + 1
    local messageLabel = UI:getControl("lblMessage")
    messageLabel:setProperty("text", "Hello, LuaUI!")
end

-- 退出按钮点击事件
function onExitClick()
    UI:exit()
end
```

### 3. C++主程序 (main.cpp)

初始化框架并运行应用：

```cpp
#include <LuaUI.h>

int main() {
    // 初始化LuaUI框架
    LuaUI::Initialize();
    
    // 加载XML布局
    // 加载Lua脚本
    // 创建UI控件
    
    // 运行主循环
    // App::run();
    
    // 关闭LuaUI框架
    LuaUI::Shutdown();
    
    return 0;
}
```

## 运行示例

1. 编译项目：
```bash
cd build
cmake --build . --config Release
```

2. 运行程序：
```bash
cd bin/Release
HelloWorld.exe
```

## 功能说明

- 点击"Click Me"按钮会显示不同的消息
- 点击"Exit"按钮会退出应用程序
- 所有逻辑都通过Lua脚本实现
- 界面布局通过XML定义

## 扩展

您可以尝试：

1. 添加更多控件到XML布局
2. 在Lua脚本中添加更多事件处理函数
3. 实现更复杂的用户交互逻辑
