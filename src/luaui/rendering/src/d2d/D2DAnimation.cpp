#include "D2DAnimation.h"
#include <cmath>

namespace luaui {
namespace rendering {

// Easing function implementations
float EaseLinear(float t) { return t; }
float EaseQuadIn(float t) { return t * t; }
float EaseQuadOut(float t) { return 1 - (1 - t) * (1 - t); }
float EaseQuadInOut(float t) {
    return t < 0.5f ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2;
}
float EaseCubicIn(float t) { return t * t * t; }
float EaseCubicOut(float t) { return 1 - std::pow(1 - t, 3); }
float EaseCubicInOut(float t) {
    return t < 0.5f ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3) / 2;
}
float EaseQuartIn(float t) { return t * t * t * t; }
float EaseQuartOut(float t) { return 1 - std::pow(1 - t, 4); }
float EaseQuartInOut(float t) {
    return t < 0.5f ? 8 * t * t * t * t : 1 - std::pow(-2 * t + 2, 4) / 2;
}
float EaseElasticIn(float t) {
    const float c4 = (2 * 3.14159265f) / 3;
    return t == 0 ? 0 : t == 1 ? 1 : -std::pow(2, 10 * t - 10) * std::sin((t * 10 - 10.75f) * c4);
}
float EaseElasticOut(float t) {
    const float c4 = (2 * 3.14159265f) / 3;
    return t == 0 ? 0 : t == 1 ? 1 : std::pow(2, -10 * t) * std::sin((t * 10 - 0.75f) * c4) + 1;
}
float EaseElasticInOut(float t) {
    const float c5 = (2 * 3.14159265f) / 4.5f;
    if (t == 0) return 0;
    if (t == 1) return 1;
    if (t < 0.5f) return -(std::pow(2, 20 * t - 10) * std::sin((20 * t - 11.125f) * c5)) / 2;
    return (std::pow(2, -20 * t + 10) * std::sin((20 * t - 11.125f) * c5)) / 2 + 1;
}
float EaseBounceOut(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    if (t < 1 / d1) {
        return n1 * t * t;
    } else if (t < 2 / d1) {
        return n1 * (t -= 1.5f / d1) * t + 0.75f;
    } else if (t < 2.5 / d1) {
        return n1 * (t -= 2.25f / d1) * t + 0.9375f;
    } else {
        return n1 * (t -= 2.625f / d1) * t + 0.984375f;
    }
}
float EaseBounceIn(float t) { return 1 - EaseBounceOut(1 - t); }
float EaseBounceInOut(float t) {
    return t < 0.5f ? (1 - EaseBounceOut(1 - 2 * t)) / 2 : (1 + EaseBounceOut(2 * t - 1)) / 2;
}
float EaseBackIn(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1;
    return c3 * t * t * t - c1 * t * t;
}
float EaseBackOut(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1;
    return 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
}
float EaseBackInOut(float t) {
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    if (t < 0.5f) return (std::pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2;
    return (std::pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
}
float EaseSineIn(float t) { return 1 - std::cos((t * 3.14159265f) / 2); }
float EaseSineOut(float t) { return std::sin((t * 3.14159265f) / 2); }
float EaseSineInOut(float t) { return -(std::cos(3.14159265f * t) - 1) / 2; }
float EaseExpoIn(float t) { return t == 0 ? 0 : std::pow(2, 10 * t - 10); }
float EaseExpoOut(float t) { return t == 1 ? 1 : 1 - std::pow(2, -10 * t); }
float EaseExpoInOut(float t) {
    return t == 0 ? 0 : t == 1 ? 1 : t < 0.5f ? std::pow(2, 20 * t - 10) / 2 : (2 - std::pow(2, -20 * t + 10)) / 2;
}
float EaseCircIn(float t) { return 1 - std::sqrt(1 - std::pow(t, 2)); }
float EaseCircOut(float t) { return std::sqrt(1 - std::pow(t - 1, 2)); }
float EaseCircInOut(float t) {
    return t < 0.5f ? (1 - std::sqrt(1 - std::pow(2 * t, 2))) / 2 
                    : (std::sqrt(1 - std::pow(-2 * t + 2, 2)) + 1) / 2;
}

float ApplyEasing(Easing type, float t) {
    // Clamp to valid range
    t = std::max(0.0f, std::min(1.0f, t));
    
    switch (type) {
        case Easing::Linear: return EaseLinear(t);
        case Easing::QuadIn: return EaseQuadIn(t);
        case Easing::QuadOut: return EaseQuadOut(t);
        case Easing::QuadInOut: return EaseQuadInOut(t);
        case Easing::CubicIn: return EaseCubicIn(t);
        case Easing::CubicOut: return EaseCubicOut(t);
        case Easing::CubicInOut: return EaseCubicInOut(t);
        case Easing::QuartIn: return EaseQuartIn(t);
        case Easing::QuartOut: return EaseQuartOut(t);
        case Easing::QuartInOut: return EaseQuartInOut(t);
        case Easing::ElasticIn: return EaseElasticIn(t);
        case Easing::ElasticOut: return EaseElasticOut(t);
        case Easing::ElasticInOut: return EaseElasticInOut(t);
        case Easing::BounceIn: return EaseBounceIn(t);
        case Easing::BounceOut: return EaseBounceOut(t);
        case Easing::BounceInOut: return EaseBounceInOut(t);
        case Easing::BackIn: return EaseBackIn(t);
        case Easing::BackOut: return EaseBackOut(t);
        case Easing::BackInOut: return EaseBackInOut(t);
        case Easing::SineIn: return EaseSineIn(t);
        case Easing::SineOut: return EaseSineOut(t);
        case Easing::SineInOut: return EaseSineInOut(t);
        case Easing::ExpoIn: return EaseExpoIn(t);
        case Easing::ExpoOut: return EaseExpoOut(t);
        case Easing::ExpoInOut: return EaseExpoInOut(t);
        case Easing::CircIn: return EaseCircIn(t);
        case Easing::CircOut: return EaseCircOut(t);
        case Easing::CircInOut: return EaseCircInOut(t);
        default: return EaseLinear(t);
    }
}

// AnimationValue implementation
AnimationValue AnimationValue::Lerp(const AnimationValue& other, float t) const {
    if (m_type != other.m_type) return *this;
    
    switch (m_type) {
        case Type::Float:
            return AnimationValue(m_float + (other.m_float - m_float) * t);
        case Type::Int:
            return AnimationValue(static_cast<int>(m_int + (other.m_int - m_int) * t));
        case Type::Bool:
            return t < 0.5f ? *this : other;
        default:
            return *this;
    }
}

// D2DAnimation implementation
D2DAnimation::D2DAnimation() = default;

void D2DAnimation::SetDuration(float milliseconds) {
    m_duration = std::max(0.0f, milliseconds);
}

float D2DAnimation::GetDuration() const {
    return m_duration;
}

void D2DAnimation::SetDelay(float milliseconds) {
    m_delay = std::max(0.0f, milliseconds);
}

float D2DAnimation::GetDelay() const {
    return m_delay;
}

void D2DAnimation::SetEasing(Easing easing) {
    m_easing = easing;
}

Easing D2DAnimation::GetEasing() const {
    return m_easing;
}

void D2DAnimation::SetIterations(int count) {
    m_iterations = count;
}

int D2DAnimation::GetIterations() const {
    return m_iterations;
}

void D2DAnimation::SetDirection(AnimationDirection direction) {
    m_direction = direction;
}

AnimationDirection D2DAnimation::GetDirection() const {
    return m_direction;
}

void D2DAnimation::SetFillMode(FillMode mode) {
    m_fillMode = mode;
}

FillMode D2DAnimation::GetFillMode() const {
    return m_fillMode;
}

void D2DAnimation::Play() {
    m_state = AnimationState::Playing;
    m_currentTime = 0;
    m_currentIteration = 0;
    m_isReversed = false;
}

void D2DAnimation::Pause() {
    if (m_state == AnimationState::Playing) {
        m_state = AnimationState::Paused;
    }
}

void D2DAnimation::Resume() {
    if (m_state == AnimationState::Paused) {
        m_state = AnimationState::Playing;
    }
}

void D2DAnimation::Stop() {
    m_state = AnimationState::Idle;
    m_currentTime = 0;
    m_currentIteration = 0;
    m_isReversed = false;
}

void D2DAnimation::Reverse() {
    m_isReversed = !m_isReversed;
    m_currentTime = m_duration - m_currentTime;
}

AnimationState D2DAnimation::GetState() const {
    return m_state;
}

bool D2DAnimation::IsRunning() const {
    return m_state == AnimationState::Playing;
}

bool D2DAnimation::IsCompleted() const {
    return m_state == AnimationState::Completed;
}

void D2DAnimation::SetProgress(float progress) {
    m_currentTime = std::max(0.0f, std::min(m_duration, progress * m_duration));
    UpdateValue();
}

float D2DAnimation::GetProgress() const {
    return m_duration > 0 ? m_currentTime / m_duration : 0;
}

void D2DAnimation::SetUpdateCallback(AnimationCallback callback) {
    m_updateCallback = callback;
}

void D2DAnimation::SetCompleteCallback(AnimationCompleteCallback callback) {
    m_completeCallback = callback;
}

void D2DAnimation::Update(float deltaTimeMs) {
    if (m_state != AnimationState::Playing) return;
    
    // Handle delay
    if (m_currentDelay < m_delay) {
        m_currentDelay += deltaTimeMs;
        if (m_currentDelay < m_delay) return;
        deltaTimeMs = m_currentDelay - m_delay;
        m_currentDelay = m_delay;
    }
    
    m_currentTime += deltaTimeMs;
    
    // Check for iteration completion
    while (m_currentTime >= m_duration) {
        m_currentTime -= m_duration;
        m_currentIteration++;
        
        // Check if animation should complete
        if (m_iterations > 0 && m_currentIteration >= m_iterations) {
            m_state = AnimationState::Completed;
            m_currentTime = m_duration;
            UpdateValue();
            if (m_completeCallback) {
                m_completeCallback();
            }
            return;
        }
        
        // Handle direction for next iteration
        if (m_direction == AnimationDirection::Alternate || 
            m_direction == AnimationDirection::AlternateReverse) {
            m_isReversed = !m_isReversed;
        }
    }
    
    UpdateValue();
}

void D2DAnimation::UpdateValue() {
    if (!m_updateCallback) return;
    
    // Calculate normalized time with easing
    float t = m_duration > 0 ? m_currentTime / m_duration : 0;
    
    // Apply direction
    if (m_isReversed || m_direction == AnimationDirection::Reverse) {
        t = 1 - t;
    }
    
    // Apply easing
    float easedT = ApplyEasing(m_easing, t);
    
    // Interpolate value
    AnimationValue currentValue = m_startValue.Lerp(m_endValue, easedT);
    m_updateCallback(currentValue);
}

// AnimationValue creation helpers
AnimationValue MakeAnimValue(float f) { return AnimationValue(f); }
AnimationValue MakeAnimValue(int i) { return AnimationValue(i); }
AnimationValue MakeAnimValue(bool b) { return AnimationValue(b); }

} // namespace rendering
} // namespace luaui
