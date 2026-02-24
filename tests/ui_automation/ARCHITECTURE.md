# LuaUI 自动化测试框架架构

## 系统架构图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           Test Scenarios                                     │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │Button Tests │ │ Layout Tests│ │MVVM Tests   │ │ Visual Reg  │           │
│  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘           │
└─────────┼───────────────┼───────────────┼───────────────┼──────────────────┘
          │               │               │               │
          └───────────────┴───────┬───────┴───────────────┘
                                  │
┌─────────────────────────────────▼──────────────────────────────────────────┐
│                     UIAutomationFramework                                   │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐             │
│  │ UITestScenario  │  │ InputSimulator  │  │ ControlFinder   │             │
│  │  (Base Class)   │  │  (Mouse/Keyboard)│  │  (Discovery)    │             │
│  └────────┬────────┘  └────────┬────────┘  └────────┬────────┘             │
│           │                    │                    │                       │
│  ┌────────▼────────┐  ┌────────▼────────┐  ┌────────▼────────┐             │
│  │    UIAssert     │  │ LayoutTester    │  │BindingTester    │             │
│  │  (Assertions)   │  │ (Validation)    │  │ (MVVM Tests)    │             │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘             │
│                                                                             │
└────────────────────────────────────────────────────────────────────────────┘
                                  │
┌─────────────────────────────────▼──────────────────────────────────────────┐
│                    Capture & Comparison Layer                               │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────┐  ┌─────────────────────┐  ┌─────────────────────┐ │
│  │ D2DOffscreenCapture │  │   PixelComparator   │  │  BaselineManager    │ │
│  │  (Screenshot API)   │  │  (Image Diff)       │  │  (Baseline Mgmt)    │ │
│  └──────────┬──────────┘  └──────────┬──────────┘  └──────────┬──────────┘ │
│             │                        │                        │            │
│             └────────────────────────┼────────────────────────┘            │
│                                      │                                     │
│  ┌───────────────────────────────────▼──────────────────────────────────┐ │
│  │                           WIC/Direct2D                               │ │
│  │              (Windows Imaging Component / Direct2D)                  │ │
│  └──────────────────────────────────────────────────────────────────────┘ │
│                                                                             │
└────────────────────────────────────────────────────────────────────────────┘
                                  │
┌─────────────────────────────────▼──────────────────────────────────────────┐
│                       LuaUI Framework Under Test                            │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌────────────┐ ┌────────────┐ ┌────────────┐ ┌────────────┐              │
│  │   Window   │ │  Controls  │ │   Layout   │ │   MVVM     │              │
│  └────────────┘ └────────────┘ └────────────┘ └────────────┘              │
│                                                                             │
└────────────────────────────────────────────────────────────────────────────┘
```

## 核心组件职责

### 1. UITestScenario (测试场景基类)
- **职责**: 提供测试生命周期管理 (SetUp/Run/TearDown)
- **功能**:
  - 自动创建/销毁测试窗口
  - 提供截图、等待等辅助方法
  - 管理测试资源

### 2. InputSimulator (输入模拟)
- **职责**: 模拟用户输入事件
- **功能**:
  - 鼠标移动、点击、拖拽、滚轮
  - 键盘按键、文本输入
  - 保持输入状态（最后鼠标位置）

### 3. ControlFinder (控件查找)
- **职责**: 在 UI 树中定位控件
- **功能**:
  - 按 ID、类型、文本查找
  - 按位置查找
  - 等待控件出现/消失

### 4. UIAssert (断言库)
- **职责**: 验证 UI 状态
- **分类**:
  - 可见性断言 (IsVisible, IsEnabled)
  - 几何断言 (HasPosition, HasSize)
  - 属性断言 (HasText, HasValue)
  - 布局断言 (IsAligned, IsContainedIn)
  - 视觉断言 (MatchesBaseline)

### 5. D2DOffscreenCapture (截图捕获)
- **职责**: 离屏渲染捕获 UI 状态
- **功能**:
  - 窗口级截图
  - 控件级截图
  - 区域截图
  - 像素数据提取

### 6. PixelComparator (像素比较)
- **职责**: 图像差异分析
- **算法**:
  - 精确比较 (Exact)
  - 模糊比较 (Fuzzy)
  - 感知比较 (Perceptual)
  - 忽略抗锯齿 (IgnoreAA)
- **输出**: 差异百分比、差异像素数、差异可视化图

### 7. BaselineManager (基线管理)
- **职责**: 管理参考图像
- **功能**:
  - 基线图像存储
  - 自动创建新基线
  - 基线更新/批准
  - 历史清理

### 8. LayoutTester (布局测试)
- **职责**: 布局正确性验证
- **功能**:
  - 布局约束验证
  - 溢出检测
  - 裁剪检测
  - 性能测量

## 数据流

### 交互测试数据流

```
┌──────────┐     ┌──────────────────┐     ┌──────────────┐     ┌──────────┐
│ Test Code│────▶│ InputSimulator   │────▶│ Window/Control│────▶│  Event   │
│          │     │ (Send mouse/key) │     │ (Process)     │     │ Handlers │
└──────────┘     └──────────────────┘     └──────────────┘     └──────────┘
      │                                                         │
      │                    ┌──────────────────┐                 │
      └───────────────────▶│    UIAssert      │◀────────────────┘
                           │ (Verify state)   │
                           └──────────────────┘
```

### 视觉回归测试数据流

```
┌──────────┐     ┌──────────────────┐     ┌──────────────┐
│ Test Code│────▶│  Capture Window  │────▶│  WIC Bitmap  │
│          │     │  (Offscreen D2D) │     │              │
└──────────┘     └──────────────────┘     └──────┬───────┘
      │                                          │
      │     ┌────────────────────────────────────┘
      │     │
      │     ▼
      │     ┌──────────────────┐     ┌──────────────┐
      │     │  Save to File    │────▶│  Output PNG  │
      │     └──────────────────┘     └──────────────┘
      │
      ▼
┌──────────────────┐     ┌──────────────┐     ┌──────────────┐
│ PixelComparator  │────▶│ Load Baseline│────▶│ Compare      │
│                  │     │              │     │ Generate Diff│
└──────────────────┘     └──────────────┘     └──────┬───────┘
                                                    │
                                              ┌─────▼──────┐
                                              │ Pass/Fail  │
                                              └────────────┘
```

## 类层次结构

```
IScreenshotCapture (Interface)
    └── D2DOffscreenCapture

UITestScenario (Abstract)
    ├── ButtonClickTest
    ├── LayoutTest
    ├── MVVM_BindingTest
    └── VisualRegressionTest

TestWindow (Window subclass for test access)
    └── Exposes protected methods for testing

AssertionError (Exception)
    └── Used for all assertion failures
```

## 扩展点

### 1. 自定义截图捕获器
```cpp
class MyCapture : public IScreenshotCapture {
    bool CaptureWindow(Window*, const std::filesystem::path&) override;
    // ...
};
```

### 2. 自定义比较算法
```cpp
PixelComparisonOptions options;
options.algorithm = ComparisonAlgorithm::Custom;
options.customComparer = [](const uint8_t* p1, const uint8_t* p2) {
    // Custom logic
    return diff;
};
```

### 3. 自定义断言
```cpp
class CustomAssert {
public:
    static void HasShadow(Control* control) {
        // Verify drop shadow exists
    }
};
```

## 测试生命周期

```
1. UIAutomationRunner::RunAll()
   │
   ├── 2. For each UITestScenario:
   │      │
   │      ├── 3. scenario->SetUp()
   │      │      ├── Create TestWindow
   │      │      ├── Create InputSimulator
     │      │      ├── Create ControlFinder
   │      │      └── Initialize Capture
   │      │
   │      ├── 4. scenario->Run()
   │      │      ├── Create UI controls
   │      │      ├── WaitForLayout()
   │      │      ├── Perform actions
   │      │      ├── Assert states
   │      │      └── Take screenshots
   │      │
   │      └── 5. scenario->TearDown()
   │             ├── Cleanup resources
   │             └── Destroy window
   │
   └── 6. Generate test report
```

## 性能考虑

### 截图优化
- 仅在必要时捕获（避免每帧捕获）
- 使用离屏渲染避免显示输出
- 异步保存大图像

### 布局等待
- 使用事件驱动而非固定延迟
- 轮询检查布局完成标志
- 超时防止无限等待

### 像素比较优化
- 先比较图像尺寸
- 使用子采样进行快速预检
- 多线程大图像比较

## 安全考虑

1. **窗口隔离**: 每个测试使用独立窗口
2. **资源清理**: TearDown 确保释放所有资源
3. **超时保护**: 所有等待操作有超时限制
4. **异常安全**: 使用 RAII 确保资源释放
