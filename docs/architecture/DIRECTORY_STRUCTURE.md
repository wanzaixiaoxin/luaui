# LuaUI 商业级目录结构

## 设计理念

1. **模块化**: 每个模块独立，职责清晰
2. **现代C++**: 头文件与源文件同目录
3. **可测试**: 测试代码与源码同级
4. **可扩展**: 预留插件、工具扩展空间

## 新目录结构

```
luaui/
├── .github/                      # CI/CD配置
│   └── workflows/
│       ├── build.yml
│       └── release.yml
│
├── cmake/                        # CMake模块
│   ├── modules/
│   │   ├── FindDirect2D.cmake
│   │   └── FindDirectWrite.cmake
│   └── utils.cmake
│
├── docs/                         # 文档
│   ├── architecture/            # 架构设计文档
│   ├── api/                     # API参考
│   ├── guides/                  # 开发指南
│   └── samples/                 # 示例教程
│
├── third_party/                 # 外部依赖（git submodule或vcpkg）
│   ├── lua/                     # Lua源码
│   ├── tinyxml2/                # XML解析
│
├── src/                         # 源代码（核心）
│   └── luaui/                   # 主命名空间目录
│       ├── core/                # 核心基础
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │       └── test_core.cpp
│       │
│       ├── platform/            # 平台抽象层
│       │   ├── include/
│       │   ├── src/
│       │   │   ├── windows/     # Windows实现
│       │   │   └── posix/       # POSIX实现(未来)
│       │   └── tests/
│       │
│       ├── rendering/           # 渲染引擎
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │
│       ├── ui/                  # UI系统
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │
│       ├── layout/              # 布局系统
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │
│       ├── input/               # 输入系统
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │
│       ├── animation/           # 动画系统
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │
│       ├── styling/             # 样式系统
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │
│       ├── binding/             # 数据绑定
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │
│       ├── scripting/           # 脚本系统
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │
│       ├── resources/           # 资源管理
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │
│       ├── xml/                 # XML解析
│       │   ├── include/
│       │   ├── src/
│       │   └── tests/
│       │
│       └── utils/               # 工具库
│           ├── include/
│           ├── src/
│           └── tests/
│
├── tools/                       # 开发工具
│   ├── designer/                # 可视化设计器
│   │   ├── src/
│   │   └── resources/
│   ├── inspector/               # UI检查器
│   └── packager/                # 资源打包工具
│
├── tests/                       # 集成测试
│   ├── integration/
│   ├── benchmarks/
│   └── fixtures/
│
├── examples/                    # 示例项目
│
├── resources/                   # 框架资源
│   └── themes/
│       ├── default.theme
│       └── dark.theme
│
├── scripts/                     # 构建脚本
│   ├── build.ps1
│   ├── build.sh
│   └── package.py
│
├── CMakeLists.txt              # 根CMake
├── LICENSE
├── README.md
└── CONTRIBUTING.md
```

## 迁移说明

### 原结构 → 新结构

| 原位置 | 新位置 | 说明 |
|--------|--------|------|
| `include/core/*.h` | `src/luaui/core/include/luaui/core/*.h` | 核心头文件 |
| `src/core/*.cpp` | `src/luaui/core/src/*.cpp` | 核心实现 |
| `include/ui/controls/*.h` | `src/luaui/ui/include/luaui/ui/*.h` | UI控件头文件 |
| `src/ui/controls/*.cpp` | `src/luaui/ui/src/controls/*.cpp` | UI控件实现 |
| `include/lua/binding/*.h` | `src/luaui/scripting/include/luaui/scripting/*.h` | 脚本绑定 |
| `src/lua/binding/*.cpp` | `src/luaui/scripting/src/bindings/*.cpp` | 绑定实现 |
| `include/xml/parser/*.h` | `src/luaui/xml/include/luaui/xml/*.h` | XML解析 |
| `src/xml/parser/*.cpp` | `src/luaui/xml/src/*.cpp` | XML实现 |
| `examples/*` | `samples/*` | 示例项目 |

## 构建系统

```cmake
# 示例：添加新模块
add_subdirectory(src/luaui/core)
add_subdirectory(src/luaui/rendering)
add_subdirectory(src/luaui/ui)
# ...

# 每个模块有自己的CMakeLists.txt
# 自动收集tests并添加测试目标
```

## 命名规范

- 目录: 小写，下划线分隔 (`layout_engine`)
- 文件: PascalCase (`RenderEngine.cpp`)
- 类名: PascalCase (`RenderEngine`)
- 命名空间: 小写 (`luaui::rendering`)
- 宏: 全大写，LUaui前缀 (`LUAUI_API`)
