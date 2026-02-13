# LuaUI 项目 - AI 编码助手指南

## 项目概述

**LuaUI** 是一个商业级的 Windows UI 框架，具有以下关键特性：

- **渲染**: 使用 Direct2D/DirectWrite 的硬件加速 2D 图形
- **布局系统**: 声明式 XML 布局（类似 XAML）配合灵活的布局容器
- **数据绑定**: 完整的 MVVM（Model-View-ViewModel）支持，包括 OneWay、TwoWay 绑定
- **语言**: 现代 C++17 标准
- **平台**: 仅 Windows（需要 Windows SDK）
- **许可证**: MIT License

## 技术栈

| 组件 | 技术 |
|-----------|------------|
| 构建系统 | CMake 3.15+ |
| 语言 | C++17 |
| 渲染 | Direct2D 1, DirectWrite, WIC |
| XML 解析 | TinyXML2 |
| 脚本 | Lua 5.4（头文件已包含） |
| Windows API | Win32, DXGI |

## 项目结构

```
luaui/
├── CMakeLists.txt              # 根 CMake 配置
├── src/luaui/                  # 主源代码
│   ├── core/                   # 核心框架（Window、Control、组件）
│   ├── controls/               # UI 控件（Button、TextBox 等）
│   │   └── layouts/            # 布局容器（StackPanel、Grid 等）
│   ├── rendering/              # 渲染引擎接口 + Direct2D 实现
│   │   └── d2d/                # Direct2D 实现
│   ├── xml/                    # XML 布局解析
│   ├── mvvm/                   # MVVM 数据绑定系统
│   ├── style/                  # 样式系统（Theme、Style、ResourceDictionary）
│   └── utils/                  # 工具类（Logger 等）
├── third_party/                # 外部依赖
│   ├── lua/                    # Lua 5.4 头文件
│   └── tinyxml2/               # XML 解析库
├── examples/                   # 示例应用（01_basic_window ~ 13_mvvm_demo）
├── tests/                      # 使用自定义测试框架的单元测试
├── docs/                       # 架构文档（主要为中文）
├── resources/                  # 框架资源
└── libs/                       # 预编译 Lua 二进制文件（lua.exe、lua54.dll）
```

## 构建说明

### 前置条件

- Windows 操作系统
- Visual Studio 2019/2022 及 C++ 工作负载
- CMake 3.15 或更高版本

### 构建命令

```bash
# 创建构建目录
mkdir build && cd build

# 配置（默认：examples ON，tests OFF）
cmake ..

# 或使用特定选项配置
cmake .. -DLUAUI_BUILD_TESTS=ON -DLUAUI_BUILD_EXAMPLES=ON

# 构建
cmake --build . --config Release

# 运行测试（如果使用 -DLUAUI_BUILD_TESTS=ON 构建）
ctest -C Release
```

### CMake 选项

| 选项 | 默认值 | 说明 |
|--------|---------|-------------|
| `LUAUI_BUILD_EXAMPLES` | ON | 构建示例应用程序 |
| `LUAUI_BUILD_TESTS` | OFF | 构建测试套件 |
| `LUAUI_BUILD_OLD_EXAMPLES` | OFF | 构建旧版示例（06-12）|

### 输出目录

- 可执行文件: `${CMAKE_BINARY_DIR}/bin/`
- 库文件: `${CMAKE_BINARY_DIR}/lib/`

## 模块架构

项目使用模块化的 CMake 结构，采用接口库：

```
LuaUI (接口库)
├── LuaUI_Core        # Window、Control、Components、Interfaces
├── LuaUI_Controls    # Button、TextBox、StackPanel、Grid 等
├── LuaUI_Rendering   # IRenderContext + Direct2D 实现
├── LuaUI_Style       # Theme、Style、ResourceDictionary
├── LuaUI_Xml         # XML 布局加载器
└── LuaUI_MVVM        # 数据绑定引擎
```

每个模块都是一个静态库，拥有自己的 `CMakeLists.txt`。

## 代码风格指南

### 命名规范

| 元素 | 规范 | 示例 |
|---------|------------|---------|
| 类 | PascalCase | `Control`, `StackPanel` |
| 函数 | PascalCase | `GetWidth()`, `SetIsVisible()` |
| 变量 | camelCase | `availableSize`, `isVisible` |
| 成员变量 | m_ 前缀 + camelCase | `m_width`, `m_parent` |
| 常量 | UPPER_CASE | `MAX_WIDTH` |
| 命名空间 | lowercase | `luaui::controls` |
| 宏 | UPPER_CASE 带前缀 | `LUAUI_API` |
| 文件 | PascalCase | `Control.h`, `StackPanel.cpp` |

### 代码格式

- **缩进**: 4 个空格（不使用 Tab）
- **注释**: 公共 API 使用 Doxygen 风格
- **包含保护**: 使用 `#pragma once`
- **换行符**: Windows 使用 CRLF

### 头文件示例模板

```cpp
#pragma once

#include "Interfaces/IControl.h"
#include <string>

namespace luaui {

/**
 * @brief 简要描述
 * @details 详细描述
 */
class Control : public interfaces::IControl {
public:
    Control();
    virtual ~Control();
    
    // Getters/Setters 遵循 PascalCase
    bool GetIsVisible() const;
    void SetIsVisible(bool visible);
    
protected:
    virtual void OnRender(rendering::IRenderContext* context);
    
private:
    std::string m_name;  // 成员变量带 m_ 前缀
    bool m_visible = true;
};

} // namespace luaui
```

## 测试框架

项目使用定义在 `tests/TestFramework.h` 中的**自定义仅头文件测试框架**。

### 编写测试

```cpp
#include "TestFramework.h"
#include "Types.h"

using namespace luaui::rendering;

TEST(Color_Construction) {
    Color c(1.0f, 0.5f, 0.25f, 0.8f);
    ASSERT_EQ(c.r, 1.0f);
    ASSERT_NEAR(c.g, 0.5f, 0.01f);
}

TEST(Color_FromHex) {
    Color c = Color::FromHex(0xFF8040);
    ASSERT_NEAR(c.r, 1.0f, 0.01f);
}

int main() {
    return RUN_ALL_TESTS();
}
```

### 可用断言

| 宏 | 说明 |
|-------|-------------|
| `TEST(name)` | 定义测试用例 |
| `ASSERT_TRUE(expr)` | 断言表达式为真 |
| `ASSERT_EQ(expected, actual)` | 断言相等 |
| `ASSERT_NE(a, b)` | 断言不相等 |
| `ASSERT_FLOAT_EQ(e, a, epsilon)` | 断言浮点数相等（带容差）|
| `ASSERT_NEAR(e, a, epsilon)` | 等同于 ASSERT_FLOAT_EQ |
| `ASSERT_NULL(ptr)` | 断言指针为空 |
| `ASSERT_NOT_NULL(ptr)` | 断言指针非空 |
| `ASSERT_THROW(expr, type)` | 断言抛出异常 |
| `ASSERT_NO_THROW(expr)` | 断言不抛出异常 |

## 关键设计模式

### 1. 基于组件的架构

控件使用组件系统来分离关注点：

```cpp
class Control {
protected:
    virtual void InitializeComponents();  // 重写以添加组件
    
private:
    components::ComponentHolder m_components;
    // LayoutComponent - 处理布局
    // RenderComponent - 处理渲染
    // InputComponent - 处理输入
};
```

### 2. MVVM 数据绑定

ViewModel 继承自 `ViewModelBase` 并实现 `INotifyPropertyChanged`：

```cpp
class MyViewModel : public luaui::mvvm::ViewModelBase {
public:
    void SetValue(int value) {
        if (m_value != value) {
            m_value = value;
            RaisePropertyChanged("Value");  // 触发 UI 更新
        }
    }
};
```

XML 绑定语法：
```xml
<TextBlock Text="{Binding UserName}" />
<TextBox Text="{Binding UserName, Mode=TwoWay}" />
```

### 3. 接口隔离

核心接口定义在 `src/luaui/core/Interfaces/`：
- `IControl` - 基本控件操作
- `IRenderable` - 渲染能力
- `ILayoutable` - 布局能力
- `IInputHandler` - 输入处理
- `IStyleable` - 样式支持

## 常见开发任务

### 添加新控件

1. 在 `src/luaui/controls/` 中创建头文件/实现文件
2. 继承自 `Control` 或 `Panel`
3. 重写 `InitializeComponents()`、`OnRender()`、`OnMeasure()`
4. 添加到 `src/luaui/controls/CMakeLists.txt`
5. 如需导出，在 `src/luaui/controls/Controls.h` 中添加
6. 在 `examples/` 中创建示例

### 添加新布局面板

1. 在 `src/luaui/controls/layouts/` 中创建文件
2. 继承自 `Panel`（带有 `PanelLayoutComponent`）
3. 在 `OnArrange()` 中实现自定义布局逻辑
4. 添加到 `layouts/CMakeLists.txt`
5. 在 `layouts/Layouts.h` 中导出

### 为控件添加 XML 支持

1. `XmlLoader` 使用 `TypeConverter` 进行属性解析
2. 在 `src/luaui/xml/TypeConverter.cpp` 中添加类型转换
3. 对于复杂控件，可能需要扩展 XML 加载器

## 重要注意事项

### 仅 Windows 平台

此框架**仅支持 Windows**。在非 Windows 系统上 CMakeLists.txt 将失败：

```cmake
if(NOT WIN32)
    message(FATAL_ERROR "LuaUI currently only supports Windows")
endif()
```

### 线程安全

- UI 操作必须在 UI 线程上执行
- 使用 `Dispatcher` 进行跨线程 UI 更新
- 控件有 `VerifyUIThread()` 用于调试

### 资源管理

- 所有控件使用 `std::shared_ptr` 进行所有权管理
- 渲染资源使用 `ResourceCache` 实现高效重用
- Direct2D 对象包装在 COM 智能指针中

### 第三方依赖

| 库 | 位置 | 用途 |
|---------|----------|-------|
| Lua 5.4 | `third_party/lua/` | 仅头文件，用于 Lua 脚本集成 |
| TinyXML2 | `third_party/tinyxml2/` | 用于布局的 XML 解析 |

### 文档语言

`docs/architecture/` 中的架构文档主要为**中文**。
关键文件：
- `docs/architecture/README.md` - 架构概览
- `docs/architecture/MVVM_DataBinding_Design.md` - MVVM 设计（中文）
- `docs/architecture/Render_Engine_Design.md` - 渲染设计（中文）

## 安全注意事项

- 目前 Lua 脚本未实现沙箱（计划在第 4 阶段实现）
- XML 布局可以引用外部资源 - 请验证路径
- 目前没有 HTTPS/网络安全功能处于活动状态

## 示例参考

| 示例 | 说明 |
|---------|-------------|
| 01_basic_window | Win32 + Direct2D 基础 |
| 02_hello_lua | Lua 集成 |
| 03_rendering_demo | 渲染引擎 |
| 04_animation_demo | 动画系统 |
| 05_offscreen_demo | 离屏渲染 |
| 06_controls_demo | 控件系统 |
| 08_visual_layout_demo | 视觉布局 |
| 12_xml_layout_demo | 带代码后置的 XML 布局 |
| 13_mvvm_demo | **MVVM 数据绑定演示** |

从构建目录运行示例：
```bash
./bin/Release/13_mvvm_demo.exe
```

---

*更多详细信息，请参阅 README.md 和 docs/architecture/ 中的架构文档。*
