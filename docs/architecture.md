# LuaUI 架构文档

## 概述

LuaUI是一个基于MFC的Windows UI框架，使用XML进行界面布局，Lua编写业务逻辑。框架采用三层架构设计，提供了良好的扩展性和易用性。

## 系统架构

### 整体架构

```
┌─────────────────────────────────────────┐
│           应用程序层 (Application)        │
│  - 主窗口管理                            │
│  - 消息循环                              │
│  - 生命周期管理                          │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│            UI层 (MFC)                    │
│  - 控件封装 (Controls)                  │
│  - 布局引擎 (Layout)                    │
│  - 事件系统 (Events)                    │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│           脚本层 (Lua)                   │
│  - Lua状态管理                           │
│  - 脚本引擎                              │
│  - API绑定                               │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│           数据层 (Data)                  │
│  - XML解析                              │
│  - 资源管理                              │
│  - 样式系统                              │
└──────────────────────────────────────────┘
```

### 核心模块

#### 1. Core模块 (src/core)

负责框架的核心功能：

- **App**: 应用程序主类，管理框架的生命周期
- **LuaState**: Lua虚拟机状态管理
- **ScriptEngine**: 脚本引擎，提供脚本加载和执行
- **Utils**: 工具函数集合（字符串、文件、编码等）

#### 2. UI模块 (src/ui)

负责UI控件的封装和管理：

- **Controls**: 所有UI控件的基类和具体实现
  - BaseControl: 控件基类
  - WindowControl: 窗口控件
  - ButtonControl: 按钮控件
  - EditControl: 文本框控件
  - LabelControl: 标签控件
  - ListControl: 列表控件
  - TreeControl: 树形控件
  - GridControl: 表格控件
  - 等等...
- **Factory**: 控件工厂，负责创建控件实例
- **Layout**: 布局引擎，处理控件的位置和大小
- **Events**: 事件系统，处理控件事件

#### 3. XML模块 (src/xml)

负责XML布局文件的解析：

- **Parser**: XML解析器，将XML解析为元素树
- **LayoutValidator**: 布局验证器，验证XML的正确性
- **ResourceParser**: 资源引用解析器，提取资源信息

#### 4. Lua模块 (src/lua)

负责Lua脚本集成：

- **Binding**: Lua C++绑定
  - LuaBinder: Lua绑定器
  - ControlBinder: 控件API绑定
  - EventBinder: 事件绑定
- **Lifecycle**: 脚本生命周期管理
  - ScriptLoader: 脚本加载器
  - ScriptRunner: 脚本执行器
  - LifecycleManager: 生命周期管理器

#### 5. Resource模块 (src/resource)

负责资源管理：

- **Loader**: 资源加载器
  - ImageLoader: 图片加载器
  - IconLoader: 图标加载器
- **Style**: 样式系统
  - StyleManager: 样式管理器
  - StyleSheet: 样式表
  - StyleEngine: 样式引擎
- **Theme**: 主题系统
  - ThemeManager: 主题管理器
  - Theme: 主题定义
  - ThemeSwitcher: 主题切换器

#### 6. Utils模块 (src/utils)

提供通用的工具函数：

- **Logger**: 日志系统
- **Config**: 配置管理
- **ErrorHandler**: 错误处理

## 数据流

### 1. 应用启动流程

```
main()
  → Initialize()
    → App::initialize()
      → ScriptEngine::initialize()
        → LuaState::initialize()
          → lua_open()
          → luaL_openlibs()
  → 加载XML布局
    → XmlParser::parseFile()
      → 创建控件树
  → 加载Lua脚本
    → ScriptEngine::loadScript()
      → LuaState::loadFile()
  → 创建UI
    → ControlFactory::createFromXml()
      → 遍历XML元素树
      → 创建对应的控件
  → 运行主循环
    → App::run()
      → MFC消息循环
```

### 2. 事件处理流程

```
用户操作
  → MFC事件捕获
    → 事件路由器 (EventRouter)
      → 查找对应控件
      → 查找事件处理器
        → LuaEventHandler
          → 调用Lua函数
            → LuaState::callFunction()
              → lua_pcall()
  → Lua函数执行
    → 修改UI属性
      → 控件更新
```

## 设计模式

### 1. 工厂模式 (Factory Pattern)

使用工厂模式创建控件实例，提供统一的创建接口：

```cpp
BaseControl* control = ControlFactory::instance().createFromXml(xmlElement);
```

### 2. 单例模式 (Singleton Pattern)

确保某些类只有一个实例：

```cpp
ControlFactory& factory = ControlFactory::instance();
```

### 3. 观察者模式 (Observer Pattern)

事件系统使用观察者模式，控件观察事件：

```cpp
control->setEventHandler("onClick", handlerRef);
```

### 4. 策略模式 (Strategy Pattern)

不同的控件类型有不同的实现策略，通过基类接口调用。

## 扩展性

### 添加新控件

1. 继承BaseControl类
2. 实现getType()、createFromXml()、bindLuaFunctions()等方法
3. 使用REGISTER_CONTROL宏注册控件

### 添加新事件

1. 在Lua绑定中注册事件名称
2. 在控件类中添加事件触发逻辑
3. 在XML Schema中定义事件属性

### 添加新布局方式

1. 继承LayoutEngine类
2. 实现calculateLayout()方法
3. 在XML中添加布局属性支持

## 性能优化

1. **Lua绑定缓存**: 缓存频繁调用的Lua函数引用
2. **控件复用**: 实现控件池机制
3. **布局计算优化**: 只在布局变更时重新计算
4. **事件批处理**: 批量处理UI事件

## 错误处理

1. **Lua错误捕获**: 使用lua_pcall捕获错误
2. **XML验证**: 使用Schema验证XML
3. **异常安全**: 确保资源正确释放
4. **日志记录**: 详细记录错误信息
