# XML Layout Demo - 声明式事件绑定

这是 LuaUI XML 布局系统的演示，展示了如何使用**声明式事件绑定**构建 UI。

## 核心特性：声明式事件绑定

传统方式需要手动查找控件并绑定事件：
```cpp
// 传统方式（繁琐）
void OnLoaded() {
    auto root = LoadXML("layout.xml");
    SetRoot(root);
    
    // 手动查找控件
    auto saveBtn = FindControl<Button>("saveBtn");
    if (saveBtn) {
        // 手动绑定事件
        saveBtn->Click.Add([this](Control*) { OnSaveClick(); });
    }
}
```

**声明式方式**（XML 中声明，代码中实现）：
```xml
<!-- XML 中声明事件 -->
<Button x:Name="saveBtn" Text="Save" Click="OnSaveClick"/>
```
```cpp
// C++ 中实现方法
void OnSaveClick() {
    UpdateStatus(L"Saved!");
}
```

## 代码结构

```
12_xml_layout_demo/
├── main.cpp              # 程序入口
├── MainWindow.h          # 主窗口类声明
├── MainWindow.cpp        # 主窗口实现
├── layouts/
│   └── main_window.xml   # UI 布局 + 事件声明
└── README.md
```

## 使用步骤

### 1. 在 XML 中声明事件

```xml
<StackPanel>
    <Button Text="New" Click="OnNewClick"/>
    <Button Text="Save" Click="OnSaveClick"/>
    <Slider ValueChanged="OnVolumeChanged"/>
</StackPanel>
```

支持的事件：
- `Click` - 按钮点击（Button）
- `ValueChanged` - 值改变（Slider, ProgressBar）
- `TextChanged` - 文本改变（TextBox）

### 2. 在 C++ 中实现事件处理器

```cpp
class MainWindow : public luaui::Window {
protected:
    void OnLoaded() override {
        auto loader = CreateXmlLoader();
        
        // 注册事件处理器
        RegisterEventHandlers(loader);
        
        // 加载 XML（事件自动绑定）
        auto root = loader->Load("layouts/main_window.xml");
        SetRoot(root);
    }
    
private:
    void RegisterEventHandlers(const std::shared_ptr<IXmlLoader>& loader) {
        loader->RegisterClickHandler("OnNewClick", [this] { OnNewClick(); });
        loader->RegisterClickHandler("OnSaveClick", [this] { OnSaveClick(); });
        loader->RegisterValueChangedHandler("OnVolumeChanged", 
            [this](double v) { OnVolumeChanged(v); });
    }
    
    // 事件处理器实现
    void OnNewClick() { /* ... */ }
    void OnSaveClick() { /* ... */ }
    void OnVolumeChanged(double value) { /* ... */ }
};
```

### 3. 运行程序

事件自动绑定，无需手动查找控件！

## 优势对比

| 特性 | 传统方式 | 声明式绑定 |
|------|----------|------------|
| 代码量 | 多（需查找+绑定） | 少（只实现方法） |
| 可读性 | 低（逻辑分散） | 高（XML 声明意图） |
| 维护性 | 差（需同步 ID） | 好（自动匹配） |
| 设计时 | 不支持 | 支持（XML 即设计） |

## XML 支持的属性

### 布局属性
- `Margin`, `Padding`, `Spacing`
- `Width`, `Height`
- `Orientation` (StackPanel)

### 外观属性
- `Background`, `Foreground`
- `FontSize`
- `SetStateColors` (Button: "normal,hover,pressed")

### 事件属性
- `Click` - 点击事件方法名
- `ValueChanged` - 值改变事件方法名
- `TextChanged` - 文本改变事件方法名

### 控件特定
- `Text` - 文本内容
- `Value`, `Minimum`, `Maximum` - 范围控件

## 扩展：自动反射绑定（未来）

理想情况下，框架可以通过反射自动发现方法：

```cpp
// 未来版本可能支持：
class MainWindow : public Window {
    // 只需要标记，无需手动注册
    void OnNewClick() { /* ... */ }
    void OnSaveClick() { /* ... */ }
};

void OnLoaded() {
    auto loader = CreateXmlLoader();
    loader->AutoBind(this);  // 自动发现所有事件方法
    SetRoot(loader->Load("layout.xml"));
}
```

当前版本需要手动注册，但已大幅简化代码。
