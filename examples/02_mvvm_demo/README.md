# MVVM Data Binding Demo

展示LuaUI的MVVM（Model-View-ViewModel）数据绑定功能。

## 已实现的功能

### ✅ 1. 声明式数据绑定（代码中实现）

```cpp
// 绑定 Status 属性到 TextBlock
auto statusText = std::make_shared<controls::TextBlock>();
BindTextProperty(statusText, "Status");

// 效果：当 ViewModel->SetStatus("Saved!") 时，
//       TextBlock 自动更新显示 "Saved!"
```

### ✅ 2. 双向绑定（滑块示例）

```cpp
// 滑块变化 -> ViewModel
ageSlider->ValueChanged.Add([this](controls::Slider*, double value) {
    m_viewModel->SetAge(static_cast<int>(value));
});

// ViewModel 变化 -> UI 更新（通过 SubscribePropertyChanged）
m_viewModel->SubscribePropertyChanged([](const PropertyChangedEventArgs& args) {
    if (args.propertyName == "Age") {
        // 更新UI显示
    }
});
```

### ✅ 3. 值转换器（FormatConverter）

```cpp
// 使用 FormatConverter 实现：
// XML: <TextBlock Text="{Binding Progress, Converter=Format, ConverterParameter='Progress: {0}%'}"/>
// 代码中：
BindWithConverter(progressLabel, "Progress", "Format", "Progress: {0}%");

// 效果：Progress = 65 显示为 "Progress: 65%"
```

### ✅ 4. ViewModel 属性通知

```cpp
class UserProfileViewModel : public ViewModelBase {
public:
    void SetProgress(double value) {
        // SetProperty 自动触发 PropertyChanged 事件
        SetProperty(m_progress, value, "Progress");
    }
    
    // 或使用便捷宏
    BINDABLE_PROPERTY(std::string, UserName)
    BINDABLE_PROPERTY(int, Age)
};
```

### ✅ 5. 命令绑定

```cpp
// 按钮点击绑定到 ViewModel 命令
saveBtn->Click.Add([this](Control*) {
    m_viewModel->SaveProfile();  // 执行命令
});
```

## 运行示例

```bash
./build/bin/Debug/02_mvvm_demo.exe
```

**交互说明：**
1. 点击 **Save** 按钮 - 触发 `SaveProfile` 命令，Status 更新为 "Saved!"
2. 点击 **+10%** 按钮 - 触发 `IncrementProgress` 命令，进度条自动增长
3. 点击 **Reset** 按钮 - 触发 `ResetProfile` 命令，所有值重置
4. 拖动 **滑块** - 双向绑定演示，Age 值同步更新

## 核心架构

```
┌─────────────────────────────────────────────────────────┐
│                         View                              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │ Status      │  │ Progress    │  │ Buttons     │     │
│  │ (TextBlock) │  │ (ProgressBar)│  │ (Commands)  │     │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘     │
│         │                │                │             │
│         │  Binding       │  Binding       │  Command    │
│         │  (OneWay)      │  (OneWay)      │             │
└─────────┼────────────────┼────────────────┼─────────────┘
          │                │                │
          ▼                ▼                ▼
┌─────────────────────────────────────────────────────────┐
│                     ViewModel                             │
│              UserProfileViewModel                         │
│  • Status: string    (INotifyPropertyChanged)            │
│  • Progress: double  (INotifyPropertyChanged)            │
│  • Age: int          (INotifyPropertyChanged)            │
│  • SaveProfile()     (Command)                           │
│  • IncrementProgress() (Command)                         │
└─────────────────────────────────────────────────────────┘
```

## 待实现（未来版本）

### 🔮 XML 声明式绑定

```xml
<Window xmlns="http://luaui.io/schema"
        DataContext="{Binding UserProfileViewModel}">
    
    <TextBlock Text="{Binding Status}"/>
    <ProgressBar Value="{Binding Progress}"/>
    <Button Command="{Binding SaveCommand}"/>
    
</Window>
```

### 🔮 自动绑定解析

```cpp
// 加载 XML 时自动解析 {Binding} 表达式
auto loader = CreateBindingXmlLoader();
loader->SetDataContext(viewModel);
auto root = loader->Load("layout.xml");  // 自动创建所有绑定
```

## 当前限制

1. **XML 绑定解析** - 框架已支持解析 `{Binding}` 表达式，但 XML Loader 尚未集成
2. **自动 TwoWay 绑定** - 需要手动实现 View->VM 的更新（通过事件监听）
3. **集合绑定** - ItemsSource 绑定尚未实现
4. **Lua ViewModel** - 计划支持用 Lua 编写 ViewModel

## 改进方向

### 短期（已具备基础，需集成）

```cpp
// 目标：一行代码创建绑定
Bind(textBlock, "Text", viewModel, BindingExpression{"Status"});

// 目标：自动解析 XML 中的绑定
loader->ConnectBindings(viewModel);
```

### 中期（需要更多开发）

```cpp
// 目标：完整的数据验证
class UserProfileViewModel : public ViewModelBase, public IDataErrorInfo {
    std::string GetError(const std::string& propertyName) const override {
        if (propertyName == "Age" && (m_age < 0 || m_age > 150))
            return "Age must be between 0 and 150";
        return "";
    }
};

// 目标：异步命令
class AsyncCommand : public ICommand {
    std::future<void> ExecuteAsync();
    bool IsExecuting() const;  // 自动禁用按钮
};
```

## 文件结构

```
02_mvvm_demo/
├── main.cpp              # 程序入口
├── MainWindow.h          # 主窗口声明
├── MainWindow.cpp        # 声明式绑定实现
├── mvvm_layouts/
│   └── main_window.xml   # XML 布局（MVVM 绑定）
└── README.md             # 本文档
```

## 总结

**已实现：**
- ✅ ViewModelBase 属性通知
- ✅ BindingEngine 单例管理
- ✅ 值转换器（Format, Boolean等）
- ✅ 命令绑定
- ✅ 代码中的声明式绑定风格

**待集成：**
- ⚠️ XML 解析绑定表达式
- ⚠️ 自动 TwoWay 绑定
- ⚠️ 集合绑定
- ⚠️ Lua ViewModel
