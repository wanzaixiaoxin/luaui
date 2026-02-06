#pragma once

#include "luaui/rendering/IAnimation.h"

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
    float m_currentTime = 0;
    int m_currentIteration = 0;
    bool m_isReversed = false;
    
    // Values
    AnimationValue m_startValue;
    AnimationValue m_endValue;
    
    // Callbacks
    AnimationCallback m_updateCallback;
    AnimationCompleteCallback m_completeCallback;
};

// Factory helpers
AnimationValue MakeAnimValue(float f);
AnimationValue MakeAnimValue(int i);
AnimationValue MakeAnimValue(bool b);

} // namespace rendering
} // namespace luaui
