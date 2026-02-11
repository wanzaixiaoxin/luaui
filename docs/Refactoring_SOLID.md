# LuaUI SOLID 重构指南

## 概述

本次重构按 SOLID 原则对 LuaUI 框架进行渐进式改进，主要目标是解决 `Control` 类过于臃肿的问题。

---

## 重构阶段

### 阶段 1: 接口层（已完成）

创建了 `src/luaui/core/include/Interfaces/` 目录，定义以下接口：

| 接口 | 职责 | 对应原 Control 方法 |
|------|------|-------------------|
| `IControl` | 基础标识和生命周期 | GetID, GetName, GetParent |
| `IRenderable` | 渲染相关 | Render, GetRenderRect, GetBackground |
| `ILayoutable` | 布局相关 | Measure, Arrange, GetWidth, SetMargin |
| `IInputHandler` | 输入处理 | OnMouseDown, OnKeyDown |
| `IFocusable` | 焦点管理 | GetIsFocusable, Focus |
| `IStyleable` | 样式和资源 | GetStyle, FindResource |
| `INativeWindow` | 平台抽象 | 窗口创建、消息循环 |

**改进收益**：
- ✅ ISP: 客户端只依赖需要的接口
- ✅ DIP: 依赖抽象而非具体实现
- ✅ 便于单元测试（可 Mock 接口）

---

### 阶段 2: 组件化（已完成）

创建了 `src/luaui/core/include/Components/` 目录：

```
Component (基类)
├── LayoutComponent  → 封装布局逻辑
├── RenderComponent  → 封装渲染逻辑
└── InputComponent   → 封装输入处理
```

**改进收益**：
- ✅ SRP: 每个组件职责单一
- ✅ 可组合：控件按需添加组件
- ✅ 可复用：组件可在不同控件间共享

---

### 阶段 3: 新 ControlBase（已完成）

新的 `ControlBase` 类结构：

```cpp
class ControlBase : public IControl {
    ComponentHolder m_components;  // 组件容器
    
    // 基础属性
    ControlID m_id;
    std::string m_name;
    bool m_visible;
    
    // 通过组件访问功能
    LayoutComponent* GetLayout() { return GetComponent<LayoutComponent>(); }
    RenderComponent* GetRender() { return GetComponent<RenderComponent>(); }
};
```

对比原 `Control` 类（447 行 → 约 100 行）：

| 指标 | 原 Control | 新 ControlBase | 改进 |
|------|-----------|---------------|------|
| 代码行数 | 447 | ~100 | -78% |
| 职责数量 | 7 | 1 | -86% |
| 虚方法数 | 25+ | 3 | -88% |
| 成员变量 | 30+ | 5 | -83% |

---

## 迁移指南

### 从旧 Control 迁移

**旧方式**（单一继承）：
```cpp
class Button : public Control {
    // 继承所有 25+ 个虚方法
    // 即使不需要键盘处理也有 OnKeyDown
};
```

**新方式**（组件组合）：
```cpp
class Button : public ControlBase {
public:
    Button() {
        // 按需添加组件
        AddComponent<RenderComponent>(this);
        AddComponent<InputComponent>(this);
        
        // 输入组件自动处理事件
        GetInput()->Click.Add(this, &Button::OnClick);
    }
    
    // 只重写需要的方法
    void OnRender(IRenderContext* ctx) override {
        // 自定义渲染
    }
};
```

---

## 架构对比

### 旧架构（继承）
```
                    Control (447行)
                         │
        ┌────────────────┼────────────────┐
        │                │                │
      Button          Panel           TextBlock
   (不需要键盘     (需要布局+渲染   (只需要渲染)
    但有方法)         但有输入)      但有输入方法)
```

### 新架构（组合）
```
               ControlBase (100行)
                    │
         ┌─────────┴─────────┐
         │         │         │
    Layout    Render    Input
   Component Component Component
         │         │         │
         └─────────┼─────────┘
                   │
        ┌──────────┼──────────┐
        │          │          │
     Button    TextBlock    Panel
   (Layout+   (Render      (Layout+
    Render+     only)        Render+
    Input)                  Input)
```

---

## 下一步工作

### 待完成

1. **实现组件源文件**
   - `LayoutComponent.cpp`
   - `RenderComponent.cpp`
   - `InputComponent.cpp`
   - `ControlBase.cpp`

2. **创建 Win32NativeWindow 实现**
   - 实现 `INativeWindow` 接口
   - 将 Win32 API 封装到具体类

3. **迁移现有控件**
   - 逐步将 Button, Panel 等迁移到新架构
   - 保持向后兼容（可同时存在两套系统）

4. **更新 CMakeLists.txt**
   - 添加新的源文件到构建系统

### 长期目标

- 完全移除旧 `Control` 类
- 支持 Linux/macOS 平台（通过 `INativeWindow` 新实现）
- 更细粒度的组件（如 AnimationComponent, EffectComponent）

---

## 兼容性说明

重构采用**渐进式策略**：
- 新接口和组件与现有代码并行存在
- 现有控件可逐步迁移
- 不影响已编译的二进制文件
