# 动画与特效系统设计

## 1. 概述

动画系统为UI提供流畅的视觉效果，支持属性动画、关键帧动画和组合动画。

## 2. 架构图

```
┌─────────────────────────────────────────────────────────────────┐
│                      动画系统架构                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                   Animation Manager                      │   │
│  │  - 全局时间管理                                          │   │
│  │  - 动画调度                                              │   │
│  │  - 性能优化                                              │   │
│  └─────────────────────────────────────────────────────────┘   │
│                              │                                  │
│          ┌───────────────────┼───────────────────┐             │
│          │                   │                   │             │
│          ▼                   ▼                   ▼             │
│  ┌───────────────┐   ┌───────────────┐   ┌───────────────┐    │
│  │   Timeline    │   │   Timeline    │   │   Timeline    │    │
│  │   (Story)     │   │  (Property)   │   │  (Keyframe)   │    │
│  └───────┬───────┘   └───────┬───────┘   └───────┬───────┘    │
│          │                   │                   │             │
│          │                   ▼                   │             │
│          │           ┌───────────────┐           │             │
│          │           │    Easing     │           │             │
│          │           │   Function    │           │             │
│          │           └───────────────┘           │             │
│          │                   │                   │             │
│          └───────────────────┼───────────────────┘             │
│                              │                                  │
│                              ▼                                  │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                     Effect System                        │   │
│  │  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌─────────┐ │   │
│  │  │   Blur    │ │  Shadow   │ │  Opacity  │ │  Color  │ │   │
│  │  │           │ │           │ │   Mask    │ │ Matrix  │ │   │
│  │  └───────────┘ └───────────┘ └───────────┘ └─────────┘ │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## 3. 核心类设计

```cpp
#pragma once
#include <chrono>
#include <functional>
#include <vector>
#include <memory>

namespace LuaUI {
namespace Animation {

// ==================== 缓动函数 ====================

enum class EasingType {
    Linear,
    // 二次
    QuadraticEaseIn,
    QuadraticEaseOut,
    QuadraticEaseInOut,
    // 三次
    CubicEaseIn,
    CubicEaseOut,
    CubicEaseInOut,
    // 四次
    QuarticEaseIn,
    QuarticEaseOut,
    QuarticEaseInOut,
    // 五次
    QuinticEaseIn,
    QuinticEaseOut,
    QuinticEaseInOut,
    // 正弦
    SineEaseIn,
    SineEaseOut,
    SineEaseInOut,
    // 指数
    ExponentialEaseIn,
    ExponentialEaseOut,
    ExponentialEaseInOut,
    // 圆形
    CircularEaseIn,
    CircularEaseOut,
    CircularEaseInOut,
    // 弹性
    ElasticEaseIn,
    ElasticEaseOut,
    ElasticEaseInOut,
    // 回弹
    BackEaseIn,
    BackEaseOut,
    BackEaseInOut,
    // 弹跳
    BounceEaseIn,
    BounceEaseOut,
    BounceEaseInOut,
    // 贝塞尔曲线
    Bezier
};

class EasingFunction {
public:
    using Func = std::function<float(float)>;
    
    static float Ease(EasingType type, float t);
    static Func GetFunction(EasingType type);
    
    // 自定义贝塞尔曲线
    static Func CubicBezier(float p0, float p1, float p2, float p3);
    
private:
    static float Linear(float t);
    static float QuadraticEaseIn(float t);
    static float QuadraticEaseOut(float t);
    static float QuadraticEaseInOut(float t);
    // ... 其他缓动函数实现
    static float ElasticEaseOut(float t);
    static float BackEaseOut(float t);
    static float BounceEaseOut(float t);
};

// ==================== 时间线基类 ====================

class Timeline : public RefCounted {
public:
    enum class State {
        Stopped,
        Running,
        Paused,
        Completed
    };
    
    enum class FillBehavior {
        HoldEnd,    // 保持结束值
        Stop        // 恢复到初始值
    };
    
    enum class RepeatBehavior {
        Count,      // 重复指定次数
        Forever,    // 无限重复
        Duration    // 在指定时间内重复
    };
    
    Timeline();
    virtual ~Timeline();
    
    // 时间属性
    void SetDuration(std::chrono::milliseconds duration);
    std::chrono::milliseconds GetDuration() const { return m_duration; }
    
    void SetBeginTime(std::chrono::milliseconds time);
    std::chrono::milliseconds GetBeginTime() const { return m_beginTime; }
    
    void SetSpeedRatio(float ratio);
    float GetSpeedRatio() const { return m_speedRatio; }
    
    // 重复行为
    void SetRepeatCount(int count);  // -1 表示无限
    int GetRepeatCount() const { return m_repeatCount; }
    
    void SetAutoReverse(bool autoReverse);
    bool GetAutoReverse() const { return m_autoReverse; }
    
    void SetFillBehavior(FillBehavior behavior);
    FillBehavior GetFillBehavior() const { return m_fillBehavior; }
    
    // 状态控制
    virtual void Begin();
    virtual void Pause();
    virtual void Resume();
    virtual void Stop();
    virtual void SkipToFill();  // 跳到结束
    virtual void Seek(std::chrono::milliseconds offset);
    
    State GetState() const { return m_state; }
    bool IsRunning() const { return m_state == State::Running; }
    bool IsCompleted() const { return m_state == State::Completed; }
    
    // 获取当前进度 (0.0 - 1.0)
    float GetProgress() const { return m_progress; }
    
    // 事件
    Event<void> Started;
    Event<void> Completed;
    Event<void> Paused;
    Event<void> Resumed;
    Event<float> ProgressChanged;  // 参数: 当前进度
    
    // 内部更新（由AnimationManager调用）
    virtual void Update(std::chrono::milliseconds currentTime);
    
protected:
    virtual void OnStarted();
    virtual void OnCompleted();
    virtual void OnUpdate(float progress) = 0;  // 子类实现
    
    std::chrono::milliseconds m_duration;
    std::chrono::milliseconds m_beginTime;
    float m_speedRatio = 1.0f;
    int m_repeatCount = 1;
    bool m_autoReverse = false;
    FillBehavior m_fillBehavior = FillBehavior::HoldEnd;
    
    State m_state = State::Stopped;
    float m_progress = 0.0f;
    std::chrono::milliseconds m_startTime;
    int m_currentIteration = 0;
    bool m_isReversing = false;
};

// ==================== 属性动画 ====================

class PropertyAnimation : public Timeline {
public:
    PropertyAnimation();
    
    // 目标
    void SetTarget(DependencyObject* target);
    DependencyObject* GetTarget() const { return m_target; }
    
    void SetProperty(const DependencyPropertyBase& property);
    const DependencyPropertyBase* GetProperty() const { return m_property; }
    
    // 值
    void SetFrom(const std::any& from);
    void SetTo(const std::any& to);
    void SetBy(const std::any& by);  // 相对值
    
    std::any GetFrom() const { return m_from; }
    std::any GetTo() const { return m_to; }
    
    // 缓动
    void SetEasingFunction(EasingType easing);
    void SetEasingFunction(const EasingFunction::Func& easing);
    
protected:
    void OnStarted() override;
    void OnUpdate(float progress) override;
    
private:
    DependencyObject* m_target = nullptr;
    const DependencyPropertyBase* m_property = nullptr;
    std::any m_from;
    std::any m_to;
    std::any m_originalValue;
    EasingFunction::Func m_easingFunction;
    
    // 类型特定的插值
    std::any InterpolateValue(const std::any& from, const std::any& to, float t);
};

// ==================== 关键帧动画 ====================

class Keyframe {
public:
    Keyframe(float keyTime, const std::any& value);
    Keyframe(float keyTime, const std::any& value, EasingType easing);
    
    float GetKeyTime() const { return m_keyTime; }
    const std::any& GetValue() const { return m_value; }
    EasingType GetEasing() const { return m_easing; }
    
private:
    float m_keyTime;  // 0.0 - 1.0
    std::any m_value;
    EasingType m_easing;
};

class KeyframeAnimation : public Timeline {
public:
    KeyframeAnimation();
    
    void SetTarget(DependencyObject* target);
    void SetProperty(const DependencyPropertyBase& property);
    
    void AddKeyframe(const Keyframe& keyframe);
    void AddKeyframe(float keyTime, const std::any& value);
    void AddKeyframe(float keyTime, const std::any& value, EasingType easing);
    
    const std::vector<Keyframe>& GetKeyframes() const { return m_keyframes; }
    
protected:
    void OnUpdate(float progress) override;
    
private:
    DependencyObject* m_target = nullptr;
    const DependencyPropertyBase* m_property = nullptr;
    std::vector<Keyframe> m_keyframes;
    
    // 查找当前进度前后的关键帧
    std::pair<const Keyframe*, const Keyframe*> FindSurroundingKeyframes(float progress);
};

// ==================== 故事板 ====================

class Storyboard : public Timeline {
public:
    Storyboard();
    
    // 添加子动画
    void AddChild(Timeline* child);
    void AddChild(Timeline* child, std::chrono::milliseconds beginTime);
    void RemoveChild(Timeline* child);
    const std::vector<RefPtr<Timeline>>& GetChildren() const { return m_children; }
    
    // 设置目标（应用于所有子动画）
    void SetTarget(DependencyObject* target);
    void SetTargetProperty(const PropertyPath& path);
    void SetTargetName(const std::string& name);
    
    // 便捷方法
    static void SetTarget(Timeline* timeline, DependencyObject* target);
    static void SetTargetProperty(Timeline* timeline, const PropertyPath& path);
    static void SetTargetName(Timeline* timeline, const std::string& name);
    
protected:
    void OnStarted() override;
    void OnCompleted() override;
    void OnUpdate(float progress) override;
    
private:
    std::vector<RefPtr<Timeline>> m_children;
    std::vector<std::chrono::milliseconds> m_childBeginTimes;
    DependencyObject* m_target = nullptr;
    PropertyPath m_targetProperty;
    std::string m_targetName;
};

// ==================== 动画管理器 ====================

class AnimationManager {
public:
    static AnimationManager& GetInstance();
    
    // 注册动画
    void Register(Timeline* timeline);
    void Unregister(Timeline* timeline);
    
    // 每帧更新（在UI线程调用）
    void Update();
    
    // 暂停/恢复所有动画
    void PauseAll();
    void ResumeAll();
    void StopAll();
    
    // 获取正在运行的动画数量
    int GetRunningCount() const;
    
    // 设置全局动画开关
    void SetAnimationsEnabled(bool enabled);
    bool AreAnimationsEnabled() const;
    
private:
    AnimationManager();
    ~AnimationManager();
    
    std::vector<RefPtr<Timeline>> m_activeTimelines;
    bool m_enabled = true;
    std::chrono::steady_clock::time_point m_lastUpdateTime;
};

// ==================== 触发器动画 ====================

class TriggerAnimation {
public:
    enum class TriggerType {
        Loaded,         // 加载完成
        MouseEnter,     // 鼠标进入
        MouseLeave,     // 鼠标离开
        MouseDown,      // 鼠标按下
        MouseUp,        // 鼠标释放
        GotFocus,       // 获得焦点
        LostFocus,      // 失去焦点
        PropertyChanged // 属性变更
    };
    
    TriggerAnimation(TriggerType type, Timeline* enterAnimation, Timeline* exitAnimation = nullptr);
    
    void Attach(UIElement* element);
    void Detach();
    
private:
    TriggerType m_type;
    RefPtr<Timeline> m_enterAnimation;
    RefPtr<Timeline> m_exitAnimation;
    UIElement* m_element = nullptr;
    std::vector<Subscription> m_subscriptions;
    
    void OnTrigger(bool entering);
};

// ==================== 过渡动画 ====================

class Transition {
public:
    enum class Type {
        Fade,
        Slide,
        Scale,
        Flip,
        Rotate
    };
    
    Transition(Type type, std::chrono::milliseconds duration);
    
    void Apply(UIElement* element, bool entering);
    
private:
    Type m_type;
    std::chrono::milliseconds m_duration;
    RefPtr<Storyboard> m_storyboard;
};

// ==================== 预定义动画资源 ====================

class AnimationResources {
public:
    // 页面过渡
    static Storyboard* FadeIn();
    static Storyboard* FadeOut();
    static Storyboard* SlideInFromLeft();
    static Storyboard* SlideInFromRight();
    static Storyboard* SlideInFromBottom();
    static Storyboard* ScaleIn();
    static Storyboard* ScaleOut();
    
    // 控件反馈
    static Storyboard* ButtonPressed();
    static Storyboard* ButtonReleased();
    static Storyboard* HighlightPulse();
    static Storyboard* Shake();
    
    // 加载动画
    static Storyboard* LoadingSpinner();
    static Storyboard* LoadingDots();
    static Storyboard* LoadingPulse();
    
    // 通知动画
    static Storyboard* NotificationSlideIn();
    static Storyboard* NotificationSlideOut();
};

}} // namespace LuaUI::Animation
```

## 4. 使用示例

### 4.1 XML中使用动画

```xml
<!-- 定义资源 -->
<Window.Resources>
    <Storyboard x:Key="FadeIn">
        <DoubleAnimation Storyboard.TargetProperty="Opacity"
                        From="0" To="1"
                        Duration="0:0:0.3"
                        EasingFunction="QuadraticEaseOut"/>
        <DoubleAnimation Storyboard.TargetProperty="(UIElement.RenderTransform).(ScaleTransform.ScaleX)"
                        From="0.95" To="1"
                        Duration="0:0:0.3"
                        EasingFunction="BackEaseOut"/>
        <DoubleAnimation Storyboard.TargetProperty="(UIElement.RenderTransform).(ScaleTransform.ScaleY)"
                        From="0.95" To="1"
                        Duration="0:0:0.3"
                        EasingFunction="BackEaseOut"/>
    </Storyboard>
    
    <Storyboard x:Key="ButtonHover">
        <DoubleAnimation Storyboard.TargetProperty="(UIElement.RenderTransform).(ScaleTransform.ScaleX)"
                        To="1.05"
                        Duration="0:0:0.15"
                        EasingFunction="QuadraticEaseOut"/>
        <DoubleAnimation Storyboard.TargetProperty="(UIElement.RenderTransform).(ScaleTransform.ScaleY)"
                        To="1.05"
                        Duration="0:0:0.15"
                        EasingFunction="QuadraticEaseOut"/>
    </Storyboard>
    
    <Storyboard x:Key="ButtonPressed">
        <DoubleAnimation Storyboard.TargetProperty="(UIElement.RenderTransform).(ScaleTransform.ScaleX)"
                        To="0.95"
                        Duration="0:0:0.05"/>
        <DoubleAnimation Storyboard.TargetProperty="(UIElement.RenderTransform).(ScaleTransform.ScaleY)"
                        To="0.95"
                        Duration="0:0:0.05"/>
    </Storyboard>
</Window.Resources>

<!-- 应用动画 -->
<Border Loaded="{StaticResource FadeIn}">
    <Button Content="Click Me">
        <Button.Triggers>
            <EventTrigger Event="MouseEnter">
                <BeginStoryboard Storyboard="{StaticResource ButtonHover}"/>
            </EventTrigger>
            <EventTrigger Event="MouseLeave">
                <BeginStoryboard Storyboard="{StaticResource ButtonNormal}"/>
            </EventTrigger>
            <EventTrigger Event="MouseDown">
                <BeginStoryboard Storyboard="{StaticResource ButtonPressed}"/>
            </EventTrigger>
        </Button.Triggers>
    </Button>
</Border>
```

### 4.2 Lua中使用动画

```lua
local UI = require("UI")
local Animation = require("UI.Animation")

-- 获取控件
local button = UI.getControl("myButton")

-- 简单动画
button:animate({
    property = "opacity",
    from = 1.0,
    to = 0.5,
    duration = 300,
    easing = Animation.Easing.EaseInOutQuad
})

-- 多属性动画
button:animate({
    properties = {
        { property = "opacity", from = 0, to = 1 },
        { property = "scaleX", from = 0.5, to = 1 },
        { property = "scaleY", from = 0.5, to = 1 },
    },
    duration = 400,
    easing = Animation.Easing.BackEaseOut
})

-- 关键帧动画
local animation = Animation.KeyframeAnimation(button, "x")
animation:addFrame(0, 0)                          -- 0ms: x = 0
animation:addFrame(0.3, 100, Animation.Easing.EaseOut)  -- 300ms: x = 100
animation:addFrame(0.7, 200, Animation.Easing.EaseIn)   -- 700ms: x = 200
animation:addFrame(1.0, 300)                      -- 1000ms: x = 300
animation:play()

-- 故事板（组合动画）
local storyboard = Animation.Storyboard()

-- 添加并行动画
storyboard:add(Animation.PropertyAnimation(button, "width", {
    to = 200,
    duration = 500,
    easing = Animation.Easing.Bounce
}))

-- 添加延迟动画
storyboard:add(Animation.PropertyAnimation(button, "background", {
    to = "#4CAF50",
    duration = 300
}), 200)  -- 延迟200ms

-- 完成回调
storyboard:onCompleted(function()
    print("Animation completed!")
end)

storyboard:begin()

-- 动画控制
storyboard:pause()
storyboard:resume()
storyboard:stop()
storyboard:seek(500)  -- 跳到500ms位置

-- 循环动画
local spinner = Animation.Storyboard()
spinner:add(Animation.PropertyAnimation(icon, "rotation", {
    from = 0,
    to = 360,
    duration = 1000,
    repeatCount = -1,  -- 无限循环
    easing = Animation.Easing.Linear
}))
spinner:begin()

-- 页面过渡
local pageContainer = UI.getControl("pageContainer")

function navigateTo(newPage)
    -- 淡出当前页面
    local fadeOut = Animation.Resources.FadeOut()
    fadeOut:setTarget(pageContainer)
    
    fadeOut:onCompleted(function()
        -- 切换页面
        pageContainer:setContent(newPage)
        
        -- 淡入新页面
        local fadeIn = Animation.Resources.SlideInFromRight()
        fadeIn:setTarget(newPage)
        fadeIn:begin()
    end)
    
    fadeOut:begin()
end
```

## 5. 特效系统

```cpp
namespace LuaUI {
namespace Effects {

// 特效基类
class Effect : public RefCounted {
public:
    virtual ~Effect() = default;
    virtual void Apply(IRenderTarget* target, const Rect& bounds) = 0;
    virtual Rect GetEffectBounds(const Rect& originalBounds) const { return originalBounds; }
};

// 模糊效果
class BlurEffect : public Effect {
public:
    explicit BlurEffect(float radius);
    
    void SetRadius(float radius);
    float GetRadius() const { return m_radius; }
    
    void SetOptimization(Optimization opt);
    
    void Apply(IRenderTarget* target, const Rect& bounds) override;
    
private:
    float m_radius;
    Optimization m_optimization;
};

// 阴影效果
class DropShadowEffect : public Effect {
public:
    DropShadowEffect(float blurRadius, const Vector2& offset, const Color& color);
    
    void SetBlurRadius(float radius);
    void SetOffset(const Vector2& offset);
    void SetColor(const Color& color);
    void SetOpacity(float opacity);
    
    void Apply(IRenderTarget* target, const Rect& bounds) override;
    Rect GetEffectBounds(const Rect& originalBounds) const override;
    
private:
    float m_blurRadius;
    Vector2 m_offset;
    Color m_color;
    float m_opacity;
};

// 颜色矩阵效果
class ColorMatrixEffect : public Effect {
public:
    // 5x4 矩阵 for RGBA transform
    void SetMatrix(const float matrix[5][4]);
    
    // 预设
    static ColorMatrixEffect* Grayscale();
    static ColorMatrixEffect* Sepia();
    static ColorMatrixEffect* Invert();
    static ColorMatrixEffect* Brightness(float amount);
    static ColorMatrixEffect* Contrast(float amount);
    static ColorMatrixEffect* Saturation(float amount);
    
    void Apply(IRenderTarget* target, const Rect& bounds) override;
    
private:
    float m_matrix[5][4];
};

// 透明遮罩
class OpacityMaskEffect : public Effect {
public:
    explicit OpacityMaskEffect(Brush* mask);
    void Apply(IRenderTarget* target, const Rect& bounds) override;
private:
    Brush* m_mask;
};

// 组合效果
class CompositeEffect : public Effect {
public:
    void AddEffect(Effect* effect);
    void Apply(IRenderTarget* target, const Rect& bounds) override;
private:
    std::vector<RefPtr<Effect>> m_effects;
};

}} // namespace LuaUI::Effects
```

## 6. 性能优化

1. **GPU加速**: 使用Direct2D效果管线
2. **离屏缓存**: 复杂效果缓存到纹理
3. **脏矩形**: 只重绘变化区域
4. **动画合并**: 相同目标的动画合并更新
5. **帧率自适应**: 根据设备性能调整动画复杂度
