# XML Layout Demo - 代码后置示例

这是 LuaUI XML 布局系统的演示，展示了如何使用外部 XML 配置文件和 C++ 代码后置（Code-Behind）模式构建 UI。

## 功能特性

### 1. 外部 XML 布局
- UI 结构定义在 `layouts/main_window.xml` 中
- 支持标准控件：StackPanel, Border, Button, TextBlock, TextBox, Slider, ProgressBar
- 使用 `x:Name` 为控件命名以便在代码中访问

### 2. 代码后置模式
- `MainWindow.h/cpp` 实现业务逻辑
- 通过 `FindNamedControls()` 方法从 XML 中查找命名控件
- 使用 `BindEvents()` 连接事件处理器

### 3. 演示的功能
- **工具栏按钮**: 新建、打开、保存
- **表单控件**: 用户名、邮箱输入框，个人简介多行文本框
- **进度显示**: 进度条
- **滑块控制**: 音量控制滑块
- **状态栏**: 显示操作反馈

## 代码结构

```
12_xml_layout_demo/
├── main.cpp              # 程序入口
├── MainWindow.h          # 主窗口类声明
├── MainWindow.cpp        # 主窗口实现（代码后置）
├── layouts/
│   └── main_window.xml   # UI 布局定义
└── README.md             # 本文档
```

## 关键代码示例

### 从 XML 加载布局
```cpp
void MainWindow::OnLoaded() {
    auto root = LoadLayoutFromXml();
    SetRoot(root);
    FindNamedControls();
    BindEvents();
}
```

### 在 XML 中定义带名称的控件
```xml
<Button x:Name="saveBtn" SetStateColors="#2196F3,#1976D2,#0D47A1"/>
<TextBox x:Name="usernameBox" Padding="10,8"/>
<Slider x:Name="volumeSlider" Minimum="0" Maximum="100" Value="75"/>
```

### 在代码后置中查找控件
```cpp
void MainWindow::FindNamedControls() {
    std::function<void(const std::shared_ptr<interfaces::IControl>&)> findNamed = 
        [&](const std::shared_ptr<interfaces::IControl>& control) {
        std::string name = control->GetName();
        if (name == "saveBtn") 
            m_saveBtn = std::dynamic_pointer_cast<Button>(control);
        // ...
    };
    findNamed(root);
}
```

### 连接事件处理器
```cpp
void MainWindow::BindEvents() {
    if (m_saveBtn) {
        m_saveBtn->Click.Add([this](Control*) {
            Logger::Info("Save button clicked!");
        });
    }
}
```

## 运行程序

```bash
# 从构建目录运行
./build/bin/Debug/12_xml_layout_demo.exe
```

确保 `layouts/main_window.xml` 位于可执行文件同级目录的 `layouts` 文件夹中。

## XML 支持的元素

当前 XML 加载器支持以下元素：
- `StackPanel` - 堆叠面板（水平/垂直）
- `Panel` - 基础面板
- `Grid` - 网格布局（基础支持，不含行列定义）
- `Border` - 边框容器
- `Button` - 按钮
- `TextBlock` - 文本显示
- `TextBox` - 文本输入
- `Slider` - 滑块
- `ProgressBar` - 进度条
- `CheckBox` - 复选框
- `ListBox` - 列表框
- `Rectangle` - 矩形
- `Ellipse` - 椭圆

## 支持的属性

### 布局属性
- `Margin` - 外边距
- `Padding` - 内边距
- `Spacing` - 子元素间距（StackPanel）
- `Width`/`Height` - 尺寸

### 外观属性
- `Background`/`Foreground` - 背景/前景色
- `BorderBrush`/`BorderThickness` - 边框

### 控件特定属性
- `x:Name` - 控件名称
- `Text` - 文本内容
- `Value`/`Minimum`/`Maximum` - 范围控件
- `SetStateColors` - 按钮三态颜色

## 构建配置

CMakeLists.txt 中配置了自动复制 layouts 目录到输出目录：

```cmake
add_custom_command(TARGET 12_xml_layout_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/12_xml_layout_demo/layouts
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/layouts
)
```

## 注意事项

1. **XML 修改后需要重新构建**才能复制到输出目录
2. **Grid 行列定义**当前版本不支持复杂的 RowDefinitions/ColumnDefinitions
3. **属性值格式**需要符合 XML 加载器的解析规则
