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

## TextBox 测试覆盖

当前示例已经扩展出一整块 `TextBox` 手测区域，覆盖以下能力：

1. `TwoWay` 绑定
   - `LiveTextBox` 与 `MirrorTextBox` 绑定到同一个 `LiveText`
   - 任意一个输入后，另一个文本框和摘要文本都应立即同步

2. ViewModel -> View 程序化更新
   - 通过“写入问候语 / 写入长文本 / 写入中英混合”按钮，验证 Lua 命令可以直接改写 `TextBox`

3. `MaxLength`
   - `LimitedTextBox` 在 `main.cpp` 中配置为 `12`
   - 手工输入和粘贴都应被截断到 12 个字符以内

4. `Password`
   - `PasswordTextBox` 在 `main.cpp` 中配置为密码模式
   - UI 应显示掩码，Lua 摘要仅显示长度与强度

5. `ReadOnly`
   - `ReadOnlyTextBox` 在 `main.cpp` 中配置为只读
   - 用户不能直接修改，但 Lua 按钮仍可替换内容

6. 长文本滚动与编辑导航
   - `ScrollTextBox` 用于手测长文本点击定位、拖拽选区、`Home/End`、`Ctrl+Arrow`、`Ctrl+Backspace/Delete`

## 手测建议

建议按以下顺序验证：

1. 在 `LiveTextBox` 输入文本，确认 `MirrorTextBox` 同步更新
2. 使用 `Ctrl+A / Ctrl+C / Ctrl+X / Ctrl+V / Ctrl+Z / Ctrl+Y`
3. 在 `LimitedTextBox` 中输入超过 12 个字符，确认被限制
4. 在 `PasswordTextBox` 中输入并点击“填充示例密码”，确认只显示掩码
5. 尝试编辑 `ReadOnlyTextBox`，确认无法改写，但可选中复制
6. 在 `ScrollTextBox` 中加载长文本后测试光标自动滚动和单词级导航

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
