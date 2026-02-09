#pragma once

#include "IAnimation.h"
#include <vector>

namespace luaui {
namespace rendering {

// D2D Animation implementation
class D2DAnimation : public IAnimation {
public:
    D2DAnimation();
    
    // IAnimation implementation
    void SetDuration(float milliseconds) override;
    float GetDuration() const override;
    
    void SetDelay(float milliseconds) override;
    float GetDelay() const override;
    
    void SetEasing(Easing easing) override;
    Easing GetEasing() const override;
    
    void SetIterations(int count) override;
    int GetIterations() const override;
    
    void SetDirection(AnimationDirection direction) override;
    AnimationDirection GetDirection() const override;
    
    void SetFillMode(FillMode mode) override;
    FillMode GetFillMode() const override;
    
    void Play() override;
    void Pause() override;
    void Resume() override;
    void Stop() override;
    void Reverse() override;
    
    AnimationState GetState() const override;
    bool IsRunning() const override;
    bool IsCompleted() const override;
    
    void SetProgress(float progress) override;
    float GetProgress() const override;
    
    void SetUpdateCallback(AnimationCallback callback) override;
    void SetCompleteCallback(AnimationCompleteCallback callback) override;
    
    void Update(float deltaTimeMs) override;
    
    // Additional methods for setting animation values
    void SetStartValue(const AnimationValue& value) { m_startValue = value; }
    void SetEndValue(const AnimationValue& value) { m_endValue = value; }
    
private:
    void UpdateValue();
    
    // Properties
    float m_duration = 1000.0f;
    float m_delay = 0.0f;
    float m_currentDelay = 0.0f;
    Easing m_easing = Easing::Linear;
    int m_iterations = 1;
    AnimationDirection m_direction = AnimationDirection::Normal;
    FillMode m_fillMode = FillMode::None;
    
    // State
    AnimationState m_state = AnimationState::Idle;
    float m_currentTime = 0.0f;
    int m_currentIteration = 0;
    bool m_isReversed = false;
    
    // Values
    AnimationValue m_startValue;
    AnimationValue m_endValue;
    
    // Callbacks
    AnimationCallback m_updateCallback;
    AnimationCompleteCallback m_completeCallback;
};

// Animation group implementation
class D2DAnimationGroup : public IAnimationGroup {
public:
    void AddAnimation(IAnimationPtr animation) override;
    void RemoveAnimation(IAnimation* animation) override;
    void Clear() override;
    
    void Play() override;
    void Pause() override;
    void Stop() override;
    
    void Update(float deltaTimeMs) override;
    bool IsComplete() const override;
    
private:
    std::vector<IAnimationPtr> m_animations;
};

// Animation timeline implementation
class D2DAnimationTimeline : public IAnimationTimeline {
public:
    // Create animations
    IAnimationPtr CreateAnimation() override;
    IAnimationGroupPtr CreateParallelGroup() override;
    IAnimationGroupPtr CreateSequentialGroup() override;
    
    // Managed animations
    void Add(IAnimationPtr animation) override;
    void Remove(IAnimation* animation) override;
    
    // Global control
    void PauseAll() override;
    void ResumeAll() override;
    void StopAll() override;
    
    // Update all animations
    void Update(float deltaTimeMs) override;
    
    // Time scale (1.0 = normal speed)
    void SetTimeScale(float scale) override;
    float GetTimeScale() const override;
    
private:
    std::vector<IAnimationPtr> m_animations;
    float m_timeScale = 1.0f;
};

// Factory helpers
AnimationValue MakeAnimValue(float f);
AnimationValue MakeAnimValue(int i);
AnimationValue MakeAnimValue(bool b);

} // namespace rendering
} // namespace luaui
