# Controls Demo (01_controls_demo) 自动化测试

## 测试覆盖范围

本测试套件为 `examples/01_controls_demo` 提供完整的自动化测试覆盖。

### 测试类别

| 测试类别 | 测试用例 | 描述 |
|---------|---------|------|
| **初始化测试** | `ControlsDemo_InitTest` | 验证窗口和控件正确加载 |
| **按钮交互** | `ControlsDemo_ButtonTest` | 测试所有按钮点击和状态更新 |
| **标签导航** | `ControlsDemo_TabNavigationTest` | 测试 TabControl 切换 |
| **滑块控件** | `ControlsDemo_SliderTest` | 测试 Slider 和 ProgressBar 同步 |
| **复选框** | `ControlsDemo_CheckBoxTest` | 测试 CheckBox 状态和事件 |
| **文本输入** | `ControlsDemo_TextBoxTest` | 测试 TextBox 输入功能 |
| **视觉回归** | `ControlsDemo_VisualRegressionTest` | 捕获所有标签页的截图 |
| **布局验证** | `ControlsDemo_LayoutValidationTest` | 验证布局正确性和性能 |

## 快速开始

### 1. 构建测试

```bash
# 从项目根目录
cd d:\Documents\learn\opensource\luaui
mkdir build && cd build
cmake .. -DLUAUI_BUILD_TESTS=ON
cmake --build . --target UIAutomationTests
```

### 2. 运行所有测试

```bash
./bin/UIAutomationTests
```

### 3. 运行特定测试

```bash
# 运行 Controls Demo 相关测试
./bin/UIAutomationTests --run ControlsDemo_InitTest
./bin/UIAutomationTests --run ControlsDemo_ButtonTest
./bin/UIAutomationTests --run ControlsDemo_TabNavigationTest
./bin/UIAutomationTests --run ControlsDemo_SliderTest
./bin/UIAutomationTests --run ControlsDemo_CheckBoxTest
./bin/UIAutomationTests --run ControlsDemo_TextBoxTest
./bin/UIAutomationTests --run ControlsDemo_VisualRegressionTest
./bin/UIAutomationTests --run ControlsDemo_LayoutValidationTest
```

### 4. 使用 CTest

```bash
# 运行所有 UI 自动化测试
ctest -R UIAutomation -V

# 运行特定测试
ctest -R ControlsDemo -V
```

## 测试详细说明

### ControlsDemo_InitTest

**目的**: 验证应用初始化正确

**测试内容**:
- 窗口标题正确显示
- TabControl 包含所有标签页
- 状态栏初始化为 "Ready"
- Basic 标签页默认可见

**预期结果**: 所有控件正确加载并可见

---

### ControlsDemo_ButtonTest

**目的**: 测试按钮交互和状态更新

**测试内容**:
- 点击 Default 按钮 → 状态栏显示 "Default button clicked"
- 点击 Primary 按钮 → 状态栏显示 "Primary button clicked"
- 点击 Success 按钮 → 状态栏显示 "Success button clicked"
- 点击 Danger 按钮 → 状态栏显示 "Danger button clicked"

**预期结果**: 每次点击正确更新状态栏

---

### ControlsDemo_TabNavigationTest

**目的**: 测试标签页切换

**测试内容**:
- 默认显示 Basic 标签页
- 切换到 Input 标签页
- 切换到 Selection 标签页
- 切换回 Basic 标签页

**预期结果**: 标签页正确切换，内容可见性正确

---

### ControlsDemo_SliderTest

**目的**: 测试 Slider 和 ProgressBar 同步

**测试内容**:
- 初始值为 50%
- 点击 75% 位置 → Slider 和 ProgressBar 更新为 75%
- 点击最小值 → 值接近 0%
- 点击最大值 → 值接近 100%

**预期结果**: Slider 和 ProgressBar 保持同步

---

### ControlsDemo_CheckBoxTest

**目的**: 测试复选框状态和事件

**测试内容**:
- Notifications 默认选中
- 取消选中 → 状态栏显示 "Notifications disabled"
- 再次选中 → 状态栏显示 "Notifications enabled"
- Auto-save 默认未选中
- 选中 Auto-save → 状态栏正确更新

**预期结果**: 复选框状态正确，事件正常触发

---

### ControlsDemo_TextBoxTest

**目的**: 测试文本输入

**测试内容**:
- 在 Username 输入框输入 "TestUser123"
- 在 Description 输入框输入 "This is a test description"

**预期结果**: 文本正确显示在输入框中

---

### ControlsDemo_VisualRegressionTest

**目的**: 捕获所有标签页的截图用于视觉回归测试

**测试内容**:
- 捕获 Basic 标签页截图
- 捕获 Input 标签页截图
- 捕获 Selection 标签页截图
- 捕获 Data 标签页截图
- 捕获 Graphics 标签页截图

**输出**: `tests/ui_automation/output/controls_demo_tab_*.png`

---

### ControlsDemo_LayoutValidationTest

**目的**: 验证布局正确性和性能

**测试内容**:
- 按钮水平排列
- 无布局违规（负尺寸、NaN 等）
- 布局性能 < 16ms（60fps 预算）

**预期结果**: 布局正确，性能达标

## 测试框架架构

```
ControlsDemo_Test.cpp
├── TestableControlsShowcase (Test Window)
│   ├── CreateBasicTab()
│   ├── CreateInputTab()
│   ├── CreateSelectionTab()
│   ├── CreateDataTab()
│   └── CreateGraphicsTab()
│
├── ControlsDemo_InitTest
├── ControlsDemo_ButtonTest
├── ControlsDemo_TabNavigationTest
├── ControlsDemo_SliderTest
├── ControlsDemo_CheckBoxTest
├── ControlsDemo_TextBoxTest
├── ControlsDemo_VisualRegressionTest
└── ControlsDemo_LayoutValidationTest
```

## 添加新测试

要添加新的测试用例，创建一个新的测试类：

```cpp
class ControlsDemo_MyNewTest : public UITestScenario {
public:
    ControlsDemo_MyNewTest() : UITestScenario("ControlsDemo_MyNewTest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestableControlsShowcase();
    }
    
    void Run() override {
        SetUp();
        WaitForLayout();
        
        // Your test code here
        {
            auto* control = m_finder->FindById("myControl");
            ASSERT_NOT_NULL(control);
            
            m_input->ClickOn(control);
            WaitForAnimation();
            
            // Assertions...
        }
        
        TearDown();
    }
};

UI_TEST(ControlsDemo_MyNewTest);
```

## 调试技巧

### 1. 在关键点截图

```cpp
void Run() override {
    SetUp();
    TakeScreenshot("step1_after_setup");
    
    // ... interact with UI
    TakeScreenshot("step2_after_interaction");
    
    TearDown();
}
```

### 2. 打印控件信息

```cpp
auto* control = m_finder->FindById("myControl");
auto rect = ControlTestHook::GetRenderRect(control);
std::cout << "Control position: (" << rect.x << ", " << rect.y << ")" << std::endl;
std::cout << "Control size: " << rect.width << "x" << rect.height << std::endl;
```

### 3. 逐步调试

```cpp
void Run() override {
    SetUp();
    
    // Add breakpoint here
    WaitForAnimation();
    
    // Interact
    m_input->ClickOn(button);
    
    // Add breakpoint here
    WaitForAnimation();
    
    TearDown();
}
```

## CI/CD 集成

### GitHub Actions

```yaml
name: Controls Demo Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-latest
    steps:
    - uses: actions/checkout@v2
    
    - name: Build
      run: |
        cmake -B build -DLUAUI_BUILD_TESTS=ON
        cmake --build build --config Release
    
    - name: Run Tests
      run: ctest -C Release -R ControlsDemo --output-on-failure
      working-directory: build
    
    - name: Upload Screenshots
      if: failure()
      uses: actions/upload-artifact@v2
      with:
        name: screenshots
        path: build/tests/ui_automation/output/
```

## 已知限制

1. **文本输入模拟**: 当前 SendText 实现可能需要根据 TextBox 实际实现调整
2. **控件查找**: 依赖 Name 属性设置，确保所有被测试的控件都有唯一名称
3. **截图捕获**: 需要窗口可见或有离屏渲染支持

## 故障排除

### 问题: 控件找不到

**解决**: 确保控件设置了 Name 属性
```cpp
auto button = std::make_shared<Button>();
button->SetName("myButton");  // 重要！
```

### 问题: 测试超时

**解决**: 增加等待时间
```cpp
WaitForLayout();  // 确保布局完成
Wait(500);        // 额外等待
```

### 问题: 截图全黑

**解决**: 确保窗口已渲染
```cpp
WaitForRender();  // 强制渲染
TakeScreenshot("name");
```
