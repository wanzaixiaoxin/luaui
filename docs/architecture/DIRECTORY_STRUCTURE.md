# LuaUI 目录结构

## 设计理念

1. **模块化**: 每个模块独立，职责清晰
2. **扁平化**: 减少路径深度，头文件直接放在 `include/` 下
3. **可测试**: 测试代码与源码同级
4. **可扩展**: 预留插件、工具扩展空间

## 实际目录结构

```
luaui/
├── .github/                      # CI/CD配置
│   └── workflows/
│       ├── build.yml
│       └── release.yml
│
├── cmake/                        # CMake模块
│   └── utils.cmake
│
├── docs/                         # 文档
│   ├── architecture/            # 架构设计文档
│   ├── api/                     # API参考
│   ├── guides/                  # 开发指南
│   └── samples/                 # 示例教程
│
├── third_party/                 # 外部依赖
│   ├── lua/                     # Lua源码
│   └── tinyxml2/                # XML解析
│
├── src/                         # 源代码
│   └── luaui/                   # 主命名空间目录
│       ├── controls/            # 控件系统
│       │   ├── include/         # 控件头文件 (Control.h, Event.h...)
│       │   ├── layout/          # 布局容器子模块
│       │   │   ├── include/     # 布局头文件 (StackPanel.h, Grid.h...)
│       │   │   └── src/         # 布局实现
│       │   └── src/             # 控件实现
│       │
│       ├── rendering/           # 渲染引擎
│       │   ├── include/         # 渲染接口 (IRenderContext.h...)
│       │   └── src/d2d/         # Direct2D实现
│       │
│       └── utils/               # 工具库
│           ├── include/         # 工具头文件 (Logger.h)
│           └── src/             # 工具实现
│
├── tools/                       # 开发工具
│   └── (预留)
│
├── tests/                       # 测试
│   ├── test_layout.cpp          # 布局系统测试
│   ├── test_logger.cpp          # 日志系统测试
│   └── test_rendering.cpp       # 渲染系统测试
│
├── examples/                    # 示例项目
│   ├── 01_basic_window          # Win32 + Direct2D基础
│   ├── 02_hello_lua             # Lua集成
│   ├── 03_rendering_demo        # 渲染引擎演示
│   ├── 04_animation_demo        # 动画系统演示
│   ├── 05_offscreen_demo        # 离屏渲染
│   ├── 06_controls_demo         # 控件系统演示
│   ├── 07_layout_demo           # 布局引擎演示
│   ├── 08_visual_layout_demo    # 可视化布局演示
│   └── 09_composite_layout      # 布局+控件复合演示
│
├── resources/                   # 框架资源
│
├── CMakeLists.txt              # 根CMake
├── LICENSE
├── README.md
└── CONTRIBUTING.md
```

## 关键模块说明

### controls - 控件系统
```
src/luaui/controls/
├── include/              # 控件头文件（扁平化，无子目录）
│   ├── Control.h        # 控件基类
│   ├── Event.h          # 事件系统
│   ├── FocusManager.h   # 焦点管理
│   ├── Shapes.h         # 形状控件 (Rectangle, Ellipse)
│   ├── CheckableControls.h  # CheckBox, RadioButton
│   └── RangeControls.h  # Slider, ProgressBar
│
├── layout/              # 布局子模块
│   ├── include/         # 布局头文件
│   │   ├── layout.h     # 统一包含头
│   │   ├── LayoutPanel.h    # Panel基类
│   │   ├── StackPanel.h     # 堆叠布局
│   │   ├── Grid.h           # 网格布局
│   │   ├── Canvas.h         # 绝对定位
│   │   ├── DockPanel.h      # 停靠布局
│   │   └── WrapPanel.h      # 自动换行
│   └── src/             # 布局实现
│
└── src/                 # 控件实现
```

### rendering - 渲染引擎
```
src/luaui/rendering/
├── include/              # 渲染接口（扁平化）
│   ├── Types.h          # 基础类型 (Size, Rect, Point, Color)
│   ├── IRenderContext.h # 渲染上下文接口
│   ├── IRenderEngine.h  # 渲染引擎接口
│   ├── IBrush.h         # 画刷接口
│   ├── IGeometry.h      # 几何接口
│   └── ...
│
└── src/d2d/             # Direct2D实现
    ├── D2DRenderEngine.cpp
    ├── D2DRenderContext.cpp
    └── ...
```

## 头文件包含规范

### 旧方式（已废弃）
```cpp
// 路径太深，不推荐
#include "luaui/controls/Control.h"
#include "luaui/rendering/Types.h"
```

### 新方式（推荐）
```cpp
// 扁平化路径，简洁
#include "Control.h"
#include "Types.h"
#include "layout.h"  // 包含所有布局容器
```

### CMake 包含目录设置
```cmake
target_include_directories(target PRIVATE
    ${CMAKE_SOURCE_DIR}/src/luaui/controls/include
    ${CMAKE_SOURCE_DIR}/src/luaui/controls/layout/include
    ${CMAKE_SOURCE_DIR}/src/luaui/rendering/include
    ${CMAKE_SOURCE_DIR}/src/luaui/utils/include
)
```

## 命名规范

- **目录**: 小写 (`controls`, `layout`, `rendering`)
- **文件**: PascalCase (`Control.h`, `StackPanel.cpp`)
- **类名**: PascalCase (`Control`, `StackPanel`, `Grid`)
- **命名空间**: 小写，嵌套 (`luaui::controls`, `luaui::rendering`)
- **宏**: 全大写，项目前缀 (`LUAUI_API`)

## 构建系统

```cmake
# 添加模块
add_subdirectory(src/luaui/utils)
add_subdirectory(src/luaui/rendering)
add_subdirectory(src/luaui/controls/layout)

# 主库自动收集所有源文件
file(GLOB_RECURSE LUAUI_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
```
