# 记事本示例

## 概述

这是一个完整的记事本应用程序示例，展示了LuaUI框架的高级功能。

## 功能特性

- **文件操作**：
  - 新建文件
  - 打开文件
  - 保存文件
  - 另存为
  - 退出

- **编辑功能**：
  - 撤销/重做
  - 剪切/复制/粘贴
  - 全选

- **格式选项**：
  - 字体选择
  - 自动换行

- **用户界面**：
  - 菜单栏
  - 工具栏
  - 状态栏
  - 多行文本编辑器

## 文件结构

```
examples/notepad/
├── main.cpp      # C++主程序
├── notepad.lua   # Lua脚本
└── layout.xml    # XML布局
```

## 代码说明

### XML布局 (layout.xml)

包含：
- 菜单栏：File、Edit、Format、Help
- 工具栏：常用操作按钮
- 编辑器：多行文本框
- 状态栏：显示状态和光标信息

```xml
<window id="mainWindow" title="Notepad - LuaUI" width="800" height="600">
    <menu id="mainMenu">...</menu>
    <toolbar id="mainToolBar">...</toolbar>
    <edit id="editText" multiline="true" x="0" y="28" width="800" height="550"/>
    <statusbar id="mainStatusBar">...</statusbar>
</window>
```

### Lua脚本 (notepad.lua)

包含完整的业务逻辑：

```lua
-- 文件操作
function menuNew_onClick()
    -- 新建文件逻辑
end

function menuSave_onClick()
    -- 保存文件逻辑
end

-- 编辑操作
function menuCut_onClick()
    -- 剪切逻辑
end

-- 辅助函数
function updateWindowTitle()
    -- 更新窗口标题
end
```

### C++主程序 (main.cpp)

初始化LuaUI框架并运行应用：

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
Notepad.exe
```

## 扩展建议

您可以尝试：

1. 添加更多编辑功能（查找、替换）
2. 实现文件打开/保存对话框
3. 添加语法高亮
4. 实现多标签页支持
5. 添加主题切换功能
6. 实现拖放文件支持

## 技术亮点

- **模块化设计**：各个功能模块独立，易于维护
- **事件驱动**：所有操作通过事件处理
- **状态管理**：完整的文件修改状态跟踪
- **用户友好**：提供撤销/重做、自动保存提示等功能
