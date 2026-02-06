# 目录结构迁移指南

## 迁移完成摘要

项目已从旧的 `include/` + `src/` 分离结构迁移到新的模块化结构。

## 新目录结构

```
src/luaui/                    # 所有源代码
├── core/                     # 核心基础
├── utils/                    # 工具库
├── platform/                 # 平台抽象
├── rendering/                # 渲染引擎
├── resources/                # 资源管理
├── xml/                      # XML解析
├── input/                    # 输入系统
├── binding/                  # 数据绑定
├── styling/                  # 样式系统
├── animation/                # 动画系统
├── layout/                   # 布局系统
├── ui/                       # UI控件
└── scripting/                # Lua脚本

samples/                      # 示例项目
├── 01_hello_world/           # Hello World示例
└── 02_notepad/               # 记事本示例

external/                     # 外部依赖 (git submodule)
tools/                        # 开发工具
tests/                        # 集成测试
docs/                         # 文档
```

## 文件迁移对照表

| 原位置 | 新位置 | 状态 |
|--------|--------|------|
| `include/core/*.h` | `src/luaui/core/include/luaui/core/*.h` | ✅ 已迁移 |
| `src/core/*.cpp` | `src/luaui/core/src/*.cpp` | ✅ 已迁移 |
| `include/ui/controls/*.h` | `src/luaui/ui/include/luaui/ui/*.h` | ✅ 已迁移 |
| `src/ui/controls/*.cpp` | `src/luaui/ui/src/controls/*.cpp` | ✅ 已迁移 |
| `include/lua/binding/*.h` | `src/luaui/scripting/include/luaui/scripting/*.h` | ✅ 已迁移 |
| `src/lua/binding/*.cpp` | `src/luaui/scripting/src/bindings/*.cpp` | ✅ 已迁移 |
| `include/xml/parser/*.h` | `src/luaui/xml/include/luaui/xml/*.h` | ✅ 已迁移 |
| `src/xml/parser/*.cpp` | `src/luaui/xml/src/*.cpp` | ✅ 已迁移 |
| `include/utils/*.h` | `src/luaui/utils/include/luaui/utils/*.h` | ✅ 已迁移 |
| `src/utils/*.cpp` | `src/luaui/utils/src/*.cpp` | ✅ 已迁移 |
| `include/resource/*.h` | `src/luaui/resources/include/luaui/resources/*.h` | ✅ 已迁移 |
| `src/resource/*.cpp` | `src/luaui/resources/src/*.cpp` | ✅ 已迁移 |
| `examples/*` | `samples/*` | ✅ 已迁移 |

## 构建方式

### 使用CMake (推荐)

```bash
# 创建构建目录
mkdir build && cd build

# 配置
cmake .. -DBUILD_TESTS=ON -DBUILD_SAMPLES=ON

# 构建
cmake --build . --config Release

# 运行测试
ctest -C Release
```

### Visual Studio

```bash
# 生成VS解决方案
cmake .. -G "Visual Studio 17 2022" -A x64

# 打开LuaUI.sln
start LuaUI.sln
```

## 添加新模块的步骤

1. 在 `src/luaui/` 下创建新目录
2. 创建 `include/luaui/<module>/` 和 `src/` 子目录
3. 创建 `CMakeLists.txt`
4. 在根 `CMakeLists.txt` 中添加 `add_subdirectory()`

## 命名规范

- **目录**: 小写，下划线分隔 (`layout_engine`)
- **文件**: PascalCase (`RenderEngine.cpp`)
- **类名**: PascalCase (`RenderEngine`)
- **命名空间**: 小写 (`luaui::rendering`)
- **函数**: camelCase (`doSomething()`)
- **变量**: camelCase (`myVariable`)
- **宏**: 全大写，LUaui前缀 (`LUAUI_API`)
- **常量**: kPascalCase (`kMaxBufferSize`)

## 旧目录清理

新结构已经建立，旧目录可以删除：

```bash
# 删除旧目录（确认迁移成功后执行）
rm -rf include/
rm -rf src/core/
rm -rf src/lua/
rm -rf src/resource/
rm -rf src/ui/
rm -rf src/utils/
rm -rf src/xml/
rm -rf examples/
```

## 后续开发建议

1. **优先实现 rendering 模块**: 这是新架构的核心
2. **逐步替换 legacy 代码**: 标记为 Legacy 的文件需要逐步替换
3. **编写单元测试**: 每个模块应有自己的测试
4. **更新文档**: 同步更新API文档
