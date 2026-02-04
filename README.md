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
