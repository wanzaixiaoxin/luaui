# LuaUI - 基于MFC的Windows UI框架

LuaUI是一个基于MFC的Windows UI框架，允许开发者使用XML进行界面布局，使用Lua编写业务逻辑。

## 特性

- **XML布局**：使用XML定义用户界面，支持嵌套容器布局
- **Lua脚本**：使用Lua编写业务逻辑，简化开发流程
- **丰富控件**：支持窗口、按钮、文本框、标签、列表、树形、表格等控件
- **事件系统**：完整的事件绑定和处理机制
- **样式主题**：支持样式和主题系统
- **资源管理**：图片、图标等资源的管理和加载

## 快速开始

### 环境要求

- **最低要求**：Visual Studio 2013 + Windows SDK 8.1
- **推荐环境**：Visual Studio 2019/2022 + CMake 3.15+
- **依赖库**：Lua 5.4.4、TinyXML2 9.0.0

### 编译项目

```bash
# 克隆仓库
git clone https://github.com/yourusername/luaui.git
cd luaui

# 使用CMake构建项目
mkdir build
cd build
cmake ..

# 编译
cmake --build . --config Release
```

### Hello World示例

创建XML布局文件 `layout.xml`：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<window id="mainWindow" title="Hello World" width="400" height="300">
    <button id="btnHello" text="Click Me" x="150" y="120" width="100" height="30"/>
    <label id="lblMessage" text="" x="150" y="160" width="100" height="20"/>
</window>
```

创建Lua脚本文件 `main.lua`：

```lua
-- 按钮点击事件处理
function onButtonClick()
    local label = UI:getControl("lblMessage")
    label:setProperty("text", "Hello, LuaUI!")
end

-- 绑定事件
UI:bindEvent("btnHello", "onClick", onButtonClick)
```

### 项目结构

```
luaui/
├── src/              # 源代码
├── include/          # 公共头文件
├── examples/         # 示例应用
├── docs/             # 文档
├── third_party/      # 第三方库
├── cmake/            # CMake模块
└── scripts/          # 构建脚本
```

## 文档

- [架构设计文档](docs/architecture.md)
- [Hello World示例](docs/examples/hello_world.md)
- [记事本示例](docs/examples/notepad.md)

## 许可证

本项目采用 Apache License 2.0 许可证 - 详见 [LICENSE](LICENSE) 文件

## 贡献

欢迎贡献代码！请查看 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详情。
[![zread](https://img.shields.io/badge/Ask_Zread-_.svg?style=flat&color=00b0aa&labelColor=000000&logo=data%3Aimage%2Fsvg%2Bxml%3Bbase64%2CPHN2ZyB3aWR0aD0iMTYiIGhlaWdodD0iMTYiIHZpZXdCb3g9IjAgMCAxNiAxNiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTQuOTYxNTYgMS42MDAxSDIuMjQxNTZDMS44ODgxIDEuNjAwMSAxLjYwMTU2IDEuODg2NjQgMS42MDE1NiAyLjI0MDFWNC45NjAxQzEuNjAxNTYgNS4zMTM1NiAxLjg4ODEgNS42MDAxIDIuMjQxNTYgNS42MDAxSDQuOTYxNTZDNS4zMTUwMiA1LjYwMDEgNS42MDE1NiA1LjMxMzU2IDUuNjAxNTYgNC45NjAxVjIuMjQwMUM1LjYwMTU2IDEuODg2NjQgNS4zMTUwMiAxLjYwMDEgNC45NjE1NiAxLjYwMDFaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00Ljk2MTU2IDEwLjM5OTlIMi4yNDE1NkMxLjg4ODEgMTAuMzk5OSAxLjYwMTU2IDEwLjY4NjQgMS42MDE1NiAxMS4wMzk5VjEzLjc1OTlDMS42MDE1NiAxNC4xMTM0IDEuODg4MSAxNC4zOTk5IDIuMjQxNTYgMTQuMzk5OUg0Ljk2MTU2QzUuMzE1MDIgMTQuMzk5OSA1LjYwMTU2IDE0LjExMzQgNS42MDE1NiAxMy43NTk5VjExLjAzOTlDNS42MDE1NiAxMC42ODY0IDUuMzE1MDIgMTAuMzk5OSA0Ljk2MTU2IDEwLjM5OTlaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik0xMy43NTg0IDEuNjAwMUgxMS4wMzg0QzEwLjY4NSAxLjYwMDEgMTAuMzk4NCAxLjg4NjY0IDEwLjM5ODQgMi4yNDAxVjQuOTYwMUMxMC4zOTg0IDUuMzEzNTYgMTAuNjg1IDUuNjAwMSAxMS4wMzg0IDUuNjAwMUgxMy43NTg0QzE0LjExMTkgNS42MDAxIDE0LjM5ODQgNS4zMTM1NiAxNC4zOTg0IDQuOTYwMVYyLjI0MDFDMTQuMzk4NCAxLjg4NjY0IDE0LjExMTkgMS42MDAxIDEzLjc1ODQgMS42MDAxWiIgZmlsbD0iI2ZmZiIvPgo8cGF0aCBkPSJNNCAxMkwxMiA0TDQgMTJaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00IDEyTDEyIDQiIHN0cm9rZT0iI2ZmZiIgc3Ryb2tlLXdpZHRoPSIxLjUiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIvPgo8L3N2Zz4K&logoColor=ffffff)](https://zread.ai/wanzaixiaoxin/luaui)