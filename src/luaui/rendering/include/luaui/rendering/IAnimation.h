#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <chrono>

namespace luaui {
namespace rendering {

// Easing functions
enum class Easing {
    Linear,
    
    // Quad
    QuadIn, QuadOut, QuadInOut,
    
    // Cubic
    CubicIn, CubicOut, CubicInOut,
    
    // Quart
    QuartIn, QuartOut, QuartInOut,
    
    // Elastic
    ElasticIn, ElasticOut, ElasticInOut,
    
    // Bounce
    BounceIn, BounceOut, BounceInOut,
    
    // Back
    BackIn, BackOut, BackInOut,
    
    // Sine
    SineIn, SineOut, SineInOut,
    
    // Expo
    ExpoIn, ExpoOut, ExpoInOut,
    
    // Circ
    CircIn, CircOut, CircInOut
};

// Animation direction
enum class AnimationDirection {
    Normal,      // Play forward
    Reverse,     // Play backward
    Alternate,   // Alternate between forward and backward
    AlternateReverse
};

// Animation fill mode
enum class FillMode {
    None,        // No style applied before/after
    Forwards,    // Retain final state
    Backwards,   // Apply initial state before start
    Both         // Both forwards and backwards
};

// Animation state
enum class AnimationState {
    Idle,
    Playing,
    Paused,
    Completed
};

// Keyframe
template<typename T>
struct Keyframe {
    float progress;  // 0.0 - 1.0
    T value;
    Easing easing;
    
    Keyframe(float p, T v, Easing e = Easing::Linear) 
        : progress(p), value(v), easing(e) {}
};

// Animation value (variant-like)
class AnimationValue {
public:
    AnimationValue() = default;
    AnimationValue(float f) : m_type(Type::Float), m_float(f) {}
    AnimationValue(int i) : m_type(Type::Int), m_int(i) {}
    AnimationValue(bool b) : m_type(Type::Bool), m_bool(b) {}
    
    float AsFloat() const { return m_float; }
    int AsInt() const { return m_int; }
    bool AsBool() const { return m_bool; }
    
    AnimationValue Lerp(const AnimationValue& other, float t) const;
    
private:
    enum class Type { Float, Int, Bool } m_type = Type::Float;
    float m_float = 0;
    int m_int = 0;
    bool m_bool = false;
};

// Animation delegate
using AnimationCallback = std::function<void(const AnimationValue&)>;
using AnimationCompleteCallback = std::function<void()>;

// Animation interface
class IAnimation {
public:
    virtual ~IAnimation() = default;
    
    // Properties
    virtual void SetDuration(float milliseconds) = 0;
    virtual float GetDuration() const = 0;
    
    virtual void SetDelay(float milliseconds) = 0;
    virtual float GetDelay() const = 0;
    
    virtual void SetEasing(Easing easing) = 0;
    virtual Easing GetEasing() const = 0;
    
    virtual void SetIterations(int count) = 0; // -1 for infinite
    virtual int GetIterations() const = 0;
    
    virtual void SetDirection(AnimationDirection direction) = 0;
    virtual AnimationDirection GetDirection() const = 0;
    
    virtual void SetFillMode(FillMode mode) = 0;
    virtual FillMode GetFillMode() const = 0;
    
    // Control
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void Stop() = 0;
    virtual void Reverse() = 0;
    
    // State
    virtual AnimationState GetState() const = 0;
    virtual bool IsRunning() const = 0;
    virtual bool IsCompleted() const = 0;
    
    // Progress (0.0 - 1.0)
    virtual void SetProgress(float progress) = 0;
    virtual float GetProgress() const = 0;
    
    // Callbacks
    virtual void SetUpdateCallback(AnimationCallback callback) = 0;
    virtual void SetCompleteCallback(AnimationCompleteCallback callback) = 0;
    
    // Update (call each frame)
    virtual void Update(float deltaTimeMs) = 0;
};

using IAnimationPtr = std::unique_ptr<IAnimation>;

// Animation group
class IAnimationGroup {
public:
    virtual ~IAnimationGroup() = default;
    
    virtual void AddAnimation(IAnimationPtr animation) = 0;
    virtual void RemoveAnimation(IAnimation* animation) = 0;
    virtual void Clear() = 0;
    
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    
    virtual void Update(float deltaTimeMs) = 0;
    virtual bool IsComplete() const = 0;
};

using IAnimationGroupPtr = std::unique_ptr<IAnimationGroup>;

// Timeline for managing multiple animations
class IAnimationTimeline {
public:
    virtual ~IAnimationTimeline() = default;
    
    // Create animations
    virtual IAnimationPtr CreateAnimation() = 0;
    virtual IAnimationGroupPtr CreateParallelGroup() = 0;
    virtual IAnimationGroupPtr CreateSequentialGroup() = 0;
    
    // Managed animations
    virtual void Add(IAnimationPtr animation) = 0;
    virtual void Remove(IAnimation* animation) = 0;
    
    // Global control
    virtual void PauseAll() = 0;
    virtual void ResumeAll() = 0;
    virtual void StopAll() = 0;
    
    // Update all animations
    virtual void Update(float deltaTimeMs) = 0;
    
    // Time scale (1.0 = normal speed)
    virtual void SetTimeScale(float scale) = 0;
    virtual float GetTimeScale() const = 0;
};

using IAnimationTimelinePtr = std::unique_ptr<IAnimationTimeline>;

// Factory
IAnimationTimelinePtr CreateAnimationTimeline();

// Easing function implementation
float ApplyEasing(Easing type, float t);

} // namespace rendering
} // namespace luaui
