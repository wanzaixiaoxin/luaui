# XML Layout Demo - 完整的代码后置示例

这是 LuaUI XML 布局系统的完整演示，展示了如何使用外部 XML 配置文件和 C++ 代码后置（Code-Behind）模式构建复杂的 UI。

## 功能特性

### 1. 外部 XML 布局
- UI 结构完全定义在 `layouts/main_window.xml` 中
- 支持所有标准控件属性（Margin、Padding、Background、Foreground 等）
- 使用 `x:Name` 为控件命名以便在代码中访问

### 2. 代码后置模式
- `MainWindow.h/cpp` 实现业务逻辑
- 通过 `FindNamedControls()` 方法从 XML 中查找命名控件
- 使用 `WireUpEvents()` 连接事件处理器

### 3. 演示的功能
- **工具栏按钮**: 新建、打开、保存、搜索
- **表单控件**: 用户名、邮箱输入框，个人简介多行文本框
- **导航栏**: 主页/个人资料切换，带视觉状态反馈
- **进度显示**: 进度条 + 百分比文本联动
- **滑块控制**: 音量控制滑块，值变化时更新状态栏
- **状态栏**: 显示操作反馈和提示信息

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

### 在 XML 中定义带名称的控件
```xml
<Button x:Name="saveBtn" Content="Save"
        SetStateColors="#107C10,#0E6E0E,#0A4F0A"
        Padding="20,10"/>
<TextBox x:Name="usernameBox" Placeholder="Enter username"/>
<Slider x:Name="volumeSlider" Minimum="0" Maximum="100" Value="75"/>
```

### 在代码后置中查找控件
```cpp
void MainWindow::FindNamedControls() {
    std::function<void(ControlPtr)> findNamed = [&](ControlPtr control) {
        std::string name = control->GetName();
        if (name == "saveBtn") 
            m_saveBtn = dynamic_cast<Button*>(control.get());
        else if (name == "volumeSlider")
            m_volumeSlider = dynamic_cast<Slider*>(control.get());
        // ...
    };
    findNamed(m_root);
}
```

### 连接事件处理器
```cpp
void MainWindow::WireUpEvents() {
    if (m_saveBtn) {
        m_saveBtn->AddClickHandler([this](Control*) { OnSaveClick(); });
    }
    if (m_volumeSlider) {
        m_volumeSlider->SetValueChangedHandler([this](Slider*, double value) {
            OnSliderValueChanged(static_cast<float>(value));
        });
    }
}
```

### 响应事件更新 UI
```cpp
void MainWindow::OnSaveClick() {
    std::wstring username = m_usernameBox ? m_usernameBox->GetText() : L"";
    UpdateStatus(L"Saved: " + username);
    UpdateProgress(100.0f);
}
```

## 运行程序

```bash
# 从构建目录运行
./build/bin/Release/12_xml_layout_demo.exe
```

确保 `layouts/main_window.xml` 位于可执行文件同级目录的 `layouts` 文件夹中。

## XML 支持的属性

本示例展示了以下 XML 属性的使用：

### 布局属性
- `Margin` - 外边距（如 "10" 或 "10,20,10,20"）
- `Padding` - 内边距
- `Spacing` - 子元素间距（StackPanel）
- `Columns`/`Rows` - 网格定义（Grid）
- `Grid.Column`/`Grid.Row` - 网格位置

### 外观属性
- `Background`/`Foreground` - 背景/前景色
- `BorderBrush`/`BorderThickness` - 边框
- `CornerRadius` - 圆角半径
- `Opacity` - 不透明度

### 控件特定属性
- `Content`/`Text` - 文本内容
- `FontSize`/`FontWeight` - 字体
- `Placeholder` - 占位符文本（TextBox）
- `IsPassword` - 密码模式（TextBox）
- `SetStateColors` - 按钮三态颜色
- `Minimum`/`Maximum`/`Value` - 范围控件

## 下一阶段的扩展

本示例为下一阶段的 MVVM 数据绑定（{Binding} 标记扩展）奠定了基础：

```xml
<!-- 即将支持的 MVVM 绑定语法 -->
<ProgressBar Value="{Binding ProfileCompletion}" Maximum="100"/>
<TextBlock Text="{Binding StatusMessage}"/>
<TextBox Text="{Binding UserName, Mode=TwoWay}"/>
```
