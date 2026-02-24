# LuaUI MVVM Demo - 03 (重构版)

## 架构说明

本示例演示了正确的 MVVM 架构分层：

```
┌─────────────────────────────────────────────────────────────┐
│ 示例代码 (03_lua_mvvm_demo)                                  │
│  - main.cpp: 仅包含 Host 初始化代码 (~150行)                  │
│  - ViewModel.lua: 业务逻辑 (Lua)                            │
│  - MainView.xml: UI 布局定义                                │
├─────────────────────────────────────────────────────────────┤
│ 框架提供 (LuaUI)                                             │
│  - lua::LuaSandbox: Lua 沙箱执行环境                        │
│  - xml::IXmlLoader: XML 布局加载器                          │
│  - mvvm::MvvmXmlLoader: MVVM 数据绑定加载器 (C++ VM)        │
│  - Window, Controls: UI 框架                                │
└─────────────────────────────────────────────────────────────┘
```

## 文件职责

### main.cpp (Host)
- 初始化 Lua 沙箱
- 加载 Lua ViewModel
- 使用框架的 XML 加载器加载 View
- **不包含任何 XML 解析或控件创建逻辑**

### ViewModel.lua (Business Logic)
- 定义数据模型
- 实现业务命令
- 使用 Log API 输出日志

### MainView.xml (View)
- 声明式 UI 布局
- 绑定表达式 `{Binding Property}`
- 命令绑定 `Command="MethodName"`

## 与 02_mvvm_demo 对比

| 特性 | 02_mvvm_demo | 03_lua_mvvm_demo |
|------|--------------|------------------|
| ViewModel 语言 | C++ | Lua |
| XML 加载器 | mvvm::MvvmXmlLoader (框架) | xml::IXmlLoader (框架) |
| 数据绑定 | 完整支持 | 基础支持 (命令绑定) |
| 代码行数 | ~200行 | ~150行 |

## 待完善的框架功能

当前 03_lua_mvvm_demo 缺少以下框架功能：

1. **Lua-aware MvvmXmlLoader**
   - 框架的 `mvvm::MvvmXmlLoader` 只支持 C++ ViewModel
   - 需要扩展支持 Lua ViewModel (`lua_State*`)
   
2. **属性绑定到 Lua**
   - 当前只实现了命令绑定
   - 需要实现 `{Binding Property}` 到 Lua 表的自动同步

3. **双向绑定**
   - Lua 属性变更 → UI 更新
   - UI 变更 → Lua 属性更新

## 正确的设计原则

### ✅ 示例应该做的：
- 使用框架提供的 API
- 专注于业务逻辑 (ViewModel.lua)
- 声明式 UI (MainView.xml)
- 最小化的 Host 代码

### ❌ 示例不应该做的：
- 实现 XML 解析器
- 实现控件创建逻辑
- 实现数据绑定引擎
- 包含框架应该提供的代码

## 参考

- `02_mvvm_demo`: 使用 C++ ViewModel 的完整 MVVM 示例
- `src/luaui/mvvm/`: 框架 MVVM 模块
- `src/luaui/lua/`: 框架 Lua 绑定模块
