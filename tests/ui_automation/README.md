# LuaUI 自动化测试方案

## 概述

本文档提供 LuaUI 框架的完整 UI 自动化测试解决方案，包括渲染验证、交互模拟、布局测试和视觉回归测试。

## 测试框架架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                    LuaUI Automation Framework                        │
├─────────────────────────────────────────────────────────────────────┤
│  Test Scenarios                                                      │
│  ├── ButtonClickTest, MVVM_BindingTest                              │
│  ├── LayoutTest, VisualRegressionTest                               │
│  └── Custom Test Scenarios                                          │
├─────────────────────────────────────────────────────────────────────┤
│  Core Components                                                     │
│  ├── UITestScenario (Base class for all tests)                      │
│  ├── InputSimulator (Mouse/Keyboard simulation)                     │
│  ├── ControlFinder (Control discovery API)                          │
│  ├── UIAssert (Assertion library)                                   │
│  ├── PixelComparator (Image comparison)                             │
│  └── LayoutTester (Layout validation)                               │
├─────────────────────────────────────────────────────────────────────┤
│  Capture & Comparison                                                │
│  ├── D2DOffscreenCapture (Headless rendering)                       │
│  ├── BaselineManager (Baseline image management)                    │
│  └── Visual Diff Generation                                         │
└─────────────────────────────────────────────────────────────────────┘
```

## 快速开始

### 1. 编写第一个 UI 测试

```cpp
#include "UIAutomationFramework.h"
#include "Button.h"
#include "TextBlock.h"
#include "Panel.h"

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::test::automation;

class MyFirstUITest : public UITestScenario {
public:
    MyFirstUITest() : UITestScenario("MyFirstUITest") {}
    
protected:
    Window* CreateTestWindow() override {
        return new TestWindow();
    }
    
    void Run() override {
        SetUp();
        
        // Create UI
        auto root = std::make_shared<StackPanel>();
        
        auto button = std::make_shared<Button>();
        button->SetText(L"Click Me");
        button->SetName("myButton");
        root->AddChild(button);
        
        // Setup window
        if (auto* tw = dynamic_cast<TestWindow*>(m_window)) {
            tw->SetRoot(root);
        }
        
        WaitForLayout();
        
        // Test: Find and click button
        auto* btn = m_finder->FindById("myButton");
        ASSERT_NOT_NULL(btn);
        ASSERT_VISIBLE(btn);
        
        m_input->ClickOn(btn);
        
        // Cleanup
        TearDown();
    }
};

UI_TEST(MyFirstUITest);
```

### 2. 运行测试

```bash
# 构建测试
cmake --build . --target UIAutomationTests

# 运行所有测试
./bin/UIAutomationTests

# 运行特定测试
./bin/UIAutomationTests --run MyFirstUITest

# 更新基线图像
./bin/UIAutomationTests --update-baselines
```

## 核心 API 参考

### InputSimulator - 输入模拟

```cpp
// 鼠标操作
m_input->MouseMove(x, y);
m_input->MouseMoveToControl(control);
m_input->MouseClick(0);  // 0=left, 1=right, 2=middle
m_input->MouseDoubleClick(0);
m_input->ClickAt(x, y);
m_input->ClickOn(control);
m_input->Drag(fromX, fromY, toX, toY);
m_input->MouseWheel(delta);

// 键盘操作
m_input->KeyPress(VK_RETURN);
m_input->SendText(L"Hello World");
```

### ControlFinder - 控件查找

```cpp
// 通过 ID 查找
auto* button = m_finder->FindById("submitButton");

// 通过类型查找
auto* textBox = m_finder->FindByType<TextBox>();

// 通过文本查找
auto* label = m_finder->FindByText(L"User Name:");

// 通过位置查找
auto* control = m_finder->FindAtPosition(100, 50);

// 等待控件出现
auto* dynamicControl = m_finder->WaitForId("loadingComplete", 5000);
```

### UIAssert - 断言

```cpp
// 可见性断言
ASSERT_VISIBLE(control);
ASSERT_NOT_VISIBLE(control);

// 位置和大小断言
ASSERT_CONTROL_AT(control, 100, 50);
ASSERT_CONTROL_SIZE(control, 200, 30);

// 属性断言
ASSERT_TEXT_EQUALS(textBlock, L"Expected Text");
UIAssert::HasValue(slider, 50.0, 0.1);

// 布局断言
UIAssert::IsAligned(control1, control2, "right");
UIAssert::IsContainedIn(child, parent);
UIAssert::DoesOverlap(control1, control2, true);

// 视觉回归断言
ASSERT_MATCHES_BASELINE(window, "homepage_baseline");
```

### 截图和像素比较

```cpp
// 捕获截图
TakeScreenshot("current_state");

// 与基线比较
PixelComparisonOptions options;
options.threshold = 0.01f;  // 1% difference allowed
options.algorithm = ComparisonAlgorithm::Fuzzy;

auto result = PixelComparator::Compare(
    "baselines/expected.png",
    "output/current.png",
    options
);

ASSERT_TRUE(result.matches);
```

## 测试类型示例

### 1. 交互测试

```cpp
class LoginFormTest : public UITestScenario {
    void Run() override {
        SetUp();
        
        // Setup login form
        // ...
        
        // Test valid login
        auto* username = m_finder->FindById("username");
        auto* password = m_finder->FindById("password");
        auto* submit = m_finder->FindById("submit");
        
        m_input->ClickOn(username);
        m_input->SendText(L"testuser");
        
        m_input->ClickOn(password);
        m_input->SendText(L"password123");
        
        m_input->ClickOn(submit);
        
        // Wait for navigation
        auto* welcome = m_finder->WaitForId("welcomeMessage", 5000);
        ASSERT_NOT_NULL(welcome);
        
        TearDown();
    }
};
```

### 2. MVVM 绑定测试

```cpp
class BindingTest : public UITestScenario {
    void Run() override {
        SetUp();
        
        auto viewModel = std::make_shared<MyViewModel>();
        
        // Bind ViewModel to View
        // ...
        
        // Test OneWay binding
        viewModel->SetUserName(L"New Name");
        WaitForBindingUpdate();
        
        auto* nameLabel = m_finder->FindById("nameLabel");
        ASSERT_TEXT_EQUALS(dynamic_cast<TextBlock*>(nameLabel), L"New Name");
        
        // Test TwoWay binding
        auto* input = m_finder->FindById("nameInput");
        m_input->ClickOn(input);
        m_input->SendText(L"Typed Name");
        
        ASSERT_EQ(viewModel->GetUserName(), L"Typed Name");
        
        TearDown();
    }
};
```

### 3. 布局测试

```cpp
class GridLayoutTest : public UITestScenario {
    void Run() override {
        SetUp();
        
        auto grid = std::make_shared<Grid>();
        // Setup grid with controls...
        
        WaitForLayout();
        
        // Validate layout
        auto* cell1 = m_finder->FindById("cell1");
        auto* cell2 = m_finder->FindById("cell2");
        
        UIAssert::IsAligned(cell2, cell1, "right");
        UIAssert::HasSize(cell1, 200, 100);
        
        // Performance test
        auto perf = LayoutTester::MeasurePerformance(m_window);
        ASSERT_LT(perf.measureTimeMs + perf.arrangeTimeMs, 16.0);  // < 16ms
        
        TearDown();
    }
};
```

### 4. 视觉回归测试

```cpp
class VisualRegressionTest : public UITestScenario {
    void Run() override {
        SetUp();
        
        // Setup UI
        // ...
        
        WaitForLayout();
        
        // Capture and compare
        TakeScreenshot("current_state");
        
        auto config = UIAutomationRunner::Instance().GetConfig();
        BaselineManager baselineManager(config.screenshotBaselinePath);
        
        auto result = baselineManager.CompareWithBaseline(
            "current_state",
            config.screenshotOutputPath / "current_state.png"
        );
        
        if (!result.matches) {
            std::cout << "Visual diff: " << result.diffImagePath << std::endl;
            ASSERT_TRUE(false);
        }
        
        TearDown();
    }
};
```

## 配置选项

```cpp
AutomationConfig config;

// Screenshot settings
config.screenshotBaselinePath = "tests/ui_automation/baselines";
config.screenshotOutputPath = "tests/ui_automation/output";
config.pixelDiffThreshold = 0.01f;  // 1%

// Timing settings
config.defaultTimeoutMs = 5000;
config.animationWaitMs = 300;
config.inputDelayMs = 50;

// Window settings
config.testWindowWidth = 800;
config.testWindowHeight = 600;
config.headlessMode = true;  // No visible window

UIAutomationRunner::Instance().SetConfig(config);
```

## CI/CD 集成

### GitHub Actions 示例

```yaml
name: UI Tests

on: [push, pull_request]

jobs:
  ui-tests:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Setup MSVC
      uses: microsoft/setup-msbuild@v1
    
    - name: Configure CMake
      run: cmake -B build -DLUAUI_BUILD_TESTS=ON
    
    - name: Build
      run: cmake --build build --config Release
    
    - name: Run UI Tests
      run: ctest -C Release -R UIAutomation --output-on-failure
    
    - name: Upload Screenshots
      uses: actions/upload-artifact@v2
      if: failure()
      with:
        name: screenshots
        path: build/tests/ui_automation/output/
```

### Azure DevOps 示例

```yaml
trigger:
  - main

pool:
  vmImage: 'windows-latest'

steps:
- task: CMake@1
  inputs:
    cmakeArgs: '-DLUAUI_BUILD_TESTS=ON'

- task: MSBuild@1
  inputs:
    solution: '**/*.sln'
    configuration: 'Release'

- script: ctest -C Release -R UIAutomation
  displayName: 'Run UI Tests'

- task: PublishTestResults@2
  inputs:
    testResultsFormat: 'JUnit'
    testResultsFiles: '**/test-results.xml'

- task: PublishBuildArtifacts@1
  condition: failed()
  inputs:
    pathToPublish: 'tests/ui_automation/output'
    artifactName: 'screenshots'
```

## 最佳实践

### 1. 测试设计原则

- **独立性**：每个测试应该是独立的，不依赖其他测试的顺序
- **确定性**：给定相同的输入，测试应该产生相同的结果
- **快速**：单个测试应该在几秒钟内完成
- **聚焦**：每个测试只验证一个概念

### 2. 选择器策略

```cpp
// Good: 使用稳定的 ID
auto* button = m_finder->FindById("submitButton");

// Avoid: 依赖文本内容（可能随语言变化）
auto* button = m_finder->FindByText(L"Submit");

// Good: 使用层次结构
auto* form = m_finder->FindById("loginForm");
auto* inputs = m_finder->FindChildren(form);
```

### 3. 等待策略

```cpp
// Good: 显式等待条件
auto* element = m_finder->WaitForId("loaded", 5000);
ASSERT_NOT_NULL(element);

// Avoid: 固定等待时间
Wait(5000);  // Too slow if element appears sooner
```

### 4. 视觉回归测试

- 建立稳定的基线图像
- 定期更新基线（UI 有意更改时）
- 忽略动态内容（时间、随机数据）
- 使用模糊匹配处理抗锯齿差异

## 故障排除

### 常见问题

1. **控件找不到**
   - 检查控件 Name 是否正确设置
   - 确保 WaitForLayout() 已调用
   - 检查控件是否被其他控件遮挡

2. **截图全黑**
   - 确保窗口已渲染（调用 Render()）
   - 检查 D2D 设备是否初始化

3. **测试不稳定**
   - 增加等待时间
   - 检查是否有异步操作
   - 添加重试逻辑

### 调试技巧

```cpp
// 在关键点截图
TakeScreenshot("debug_step_1");

// 打印控件信息
auto* control = m_finder->FindById("myControl");
auto rect = ControlTestHook::GetRenderRect(control);
std::cout << "Control at: " << rect.x << ", " << rect.y 
          << " size: " << rect.width << "x" << rect.height << std::endl;
```

## 扩展框架

### 自定义断言

```cpp
class CustomUIAssert {
public:
    static void HasColor(Control* control, Color expected) {
        // Implementation
    }
    
    static void IsInViewport(Control* control) {
        auto rect = ControlTestHook::GetRenderRect(control);
        // Check if within window bounds
    }
};
```

### 自定义截图捕获

```cpp
class CustomCapture : public IScreenshotCapture {
    bool CaptureWindow(Window* window, const std::filesystem::path& path) override {
        // Custom implementation
    }
    // ...
};
```

## 总结

本自动化测试框架提供：

- ✅ **完整的交互模拟**：鼠标、键盘事件
- ✅ **控件查找 API**：ID、类型、文本、位置
- ✅ **丰富的断言库**：可见性、位置、大小、内容
- ✅ **视觉回归测试**：像素级比较、基线管理
- ✅ **布局验证**：对齐、包含、性能
- ✅ **MVVM 测试**：绑定验证、数据流测试
- ✅ **CI/CD 集成**：GitHub Actions、Azure DevOps 支持
