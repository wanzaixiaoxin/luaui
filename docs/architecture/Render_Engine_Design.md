# 渲染引擎详细设计

## 1. 概述

渲染引擎是整个UI框架的核心，决定视觉效果和流畅度。采用 **Direct2D + DirectComposition** 技术栈。

## 2. 技术选型理由

| 技术 | 优势 | 适用场景 |
|------|------|----------|
| Direct2D | GPU加速，高质量2D渲染 | 2D图形绘制 |
| DirectWrite | 高质量文本渲染 | 字体显示 |
| DirectComposition | 窗口合成，视觉效果 | 亚克力、阴影等 |
| WIC | 图像编解码 | 图片加载 |

## 3. 渲染架构

```
┌─────────────────────────────────────────────────────────────────┐
│                      渲染管线架构                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐         │
│  │   Scene     │───▶│   Render    │───▶│   Display   │         │
│  │   Graph     │    │   Pass      │    │   Output    │         │
│  └─────────────┘    └─────────────┘    └─────────────┘         │
│         │                  │                  │                 │
│         ▼                  ▼                  ▼                 │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐         │
│  │ Visual Tree │    │ Draw Lists  │    │ Swap Chain  │         │
│  │ Dirty Rect  │    │ Batching    │    │ VSync       │         │
│  │ Culling     │    │ Effects     │    │ Present     │         │
│  └─────────────┘    └─────────────┘    └─────────────┘         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## 4. 核心接口设计

```cpp
#pragma once
#include <d2d1.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <dcomp.h>
#include <wrl/client.h>

namespace LuaUI {
namespace Render {

using Microsoft::WRL::ComPtr;

// ==================== 基础类型 ====================

struct Color {
    float r, g, b, a;
    static Color FromHex(uint32_t hex);
    static Color FromHSV(float h, float s, float v, float a = 1.0f);
    D2D1_COLOR_F ToD2D() const;
};

struct Point {
    float x, y;
    D2D1_POINT_2F ToD2D() const { return {x, y}; }
};

struct Size {
    float width, height;
    bool IsEmpty() const { return width <= 0 || height <= 0; }
    D2D1_SIZE_F ToD2D() const { return {width, height}; }
};

struct Rect {
    float x, y, width, height;
    float Left() const { return x; }
    float Top() const { return y; }
    float Right() const { return x + width; }
    float Bottom() const { return y + height; }
    bool Contains(const Point& p) const;
    bool Intersects(const Rect& other) const;
    Rect Intersect(const Rect& other) const;
    D2D1_RECT_F ToD2D() const { return {x, y, x + width, y + height}; }
};

struct Thickness {
    float left, top, right, bottom;
    Thickness(float uniform) : left(uniform), top(uniform), right(uniform), bottom(uniform) {}
    Thickness(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}
};

// ==================== 变换矩阵 ====================

class Transform {
public:
    float m[3][2];  // 3x2 矩阵 for 2D affine transform
    
    static Transform Identity();
    static Transform Translation(float x, float y);
    static Transform Scale(float sx, float sy, float cx = 0, float cy = 0);
    static Transform Rotation(float angle, float cx = 0, float cy = 0);
    static Transform Skew(float sx, float sy);
    
    Transform operator*(const Transform& other) const;
    D2D1_MATRIX_3X2_F ToD2D() const;
};

// ==================== 画刷系统 ====================

class Brush : public RefCounted {
public:
    virtual ~Brush() = default;
    virtual ID2D1Brush* GetD2DBrush(IRenderTarget* target) = 0;
};

class SolidColorBrush : public Brush {
public:
    explicit SolidColorBrush(const Color& color);
    void SetColor(const Color& color);
    ID2D1Brush* GetD2DBrush(IRenderTarget* target) override;
private:
    Color m_color;
    ComPtr<ID2D1SolidColorBrush> m_brush;
};

class LinearGradientBrush : public Brush {
public:
    struct GradientStop {
        float position;
        Color color;
    };
    
    LinearGradientBrush(const Point& start, const Point& end, 
                        const std::vector<GradientStop>& stops);
    ID2D1Brush* GetD2DBrush(IRenderTarget* target) override;
private:
    Point m_start, m_end;
    std::vector<GradientStop> m_stops;
    ComPtr<ID2D1LinearGradientBrush> m_brush;
};

class RadialGradientBrush : public Brush {
public:
    RadialGradientBrush(const Point& center, const Point& offset,
                        float radiusX, float radiusY,
                        const std::vector<LinearGradientBrush::GradientStop>& stops);
    ID2D1Brush* GetD2DBrush(IRenderTarget* target) override;
private:
    Point m_center, m_offset;
    float m_radiusX, m_radiusY;
    std::vector<LinearGradientBrush::GradientStop> m_stops;
    ComPtr<ID2D1RadialGradientBrush> m_brush;
};

class ImageBrush : public Brush {
public:
    enum class ExtendMode { Clamp, Wrap, Mirror };
    enum class Alignment { Center, TopLeft, TopRight, BottomLeft, BottomRight, Stretch };
    
    explicit ImageBrush(class Image* image);
    void SetExtendMode(ExtendMode x, ExtendMode y);
    void SetAlignment(Alignment align);
    void SetSourceRect(const Rect& rect);
    ID2D1Brush* GetD2DBrush(IRenderTarget* target) override;
private:
    class Image* m_image;
    ExtendMode m_extendX, m_extendY;
    Alignment m_alignment;
    Rect m_sourceRect;
    ComPtr<ID2D1BitmapBrush> m_brush;
};

// ==================== 文本格式 ====================

class TextFormat : public RefCounted {
public:
    enum class FontWeight { Thin = 100, ExtraLight = 200, Light = 300, 
                           Normal = 400, Medium = 500, SemiBold = 600, 
                           Bold = 700, ExtraBold = 800, Black = 900 };
    enum class FontStyle { Normal, Italic, Oblique };
    enum class TextAlignment { Leading, Trailing, Center, Justified };
    enum class ParagraphAlignment { Near, Far, Center };
    enum class WordWrapping { Wrap, NoWrap, EmergencyBreak, WholeWord };
    enum class TrimmingGranularity { None, Character, Word };
    
    TextFormat(const std::wstring& fontFamily, float fontSize);
    void SetFontWeight(FontWeight weight);
    void SetFontStyle(FontStyle style);
    void SetTextAlignment(TextAlignment align);
    void SetParagraphAlignment(ParagraphAlignment align);
    void SetWordWrapping(WordWrapping wrapping);
    IDWriteTextFormat* GetDWriteFormat();
    
private:
    std::wstring m_fontFamily;
    float m_fontSize;
    FontWeight m_weight;
    FontStyle m_style;
    TextAlignment m_textAlign;
    ParagraphAlignment m_paraAlign;
    WordWrapping m_wrapping;
    ComPtr<IDWriteTextFormat> m_format;
};

// ==================== 图像 ====================

class Image : public RefCounted {
public:
    static Image* LoadFromFile(const std::wstring& path);
    static Image* LoadFromMemory(const void* data, size_t size);
    static Image* CreateFromBitmap(int width, int height, const void* pixels);
    
    int GetWidth() const;
    int GetHeight() const;
    Size GetSize() const;
    
    ID2D1Bitmap* GetD2DBitmap(IRenderTarget* target);
    
private:
    int m_width, m_height;
    ComPtr<ID2D1Bitmap> m_bitmap;
    std::vector<uint8_t> m_pixelData;  // Fallback data
};

// ==================== 路径几何 ====================

class Path : public RefCounted {
public:
    Path();
    
    // SVG-like 路径命令
    void BeginFigure(const Point& start, bool filled = true);
    void EndFigure(bool closed = true);
    void LineTo(const Point& point);
    void BezierTo(const Point& cp1, const Point& cp2, const Point& end);
    void QuadraticBezierTo(const Point& cp, const Point& end);
    void ArcTo(const Point& end, const Size& radius, float rotation, 
               bool isLargeArc, bool sweepClockwise);
    
    // 形状快捷方法
    void AddRectangle(const Rect& rect);
    void AddRoundedRectangle(const Rect& rect, float radiusX, float radiusY);
    void AddEllipse(const Point& center, float radiusX, float radiusY);
    
    ID2D1Geometry* GetD2DGeometry(IRenderTarget* target);
    
private:
    ComPtr<ID2D1PathGeometry> m_geometry;
    ComPtr<ID2D1GeometrySink> m_sink;
    bool m_figureOpen;
};

// ==================== 渲染目标 ====================

class IRenderTarget {
public:
    virtual ~IRenderTarget() = default;
    
    // 状态管理
    virtual void PushTransform(const Transform& transform) = 0;
    virtual void PopTransform() = 0;
    virtual void PushClip(const Rect& rect) = 0;
    virtual void PushClip(const Path& path) = 0;
    virtual void PopClip() = 0;
    virtual void PushOpacity(float opacity) = 0;
    virtual void PopOpacity() = 0;
    
    // 绘制命令
    virtual void Clear(const Color& color) = 0;
    virtual void DrawLine(const Point& p1, const Point& p2, Brush* brush, float strokeWidth = 1.0f) = 0;
    virtual void DrawRectangle(const Rect& rect, Brush* brush, float strokeWidth = 1.0f) = 0;
    virtual void FillRectangle(const Rect& rect, Brush* brush) = 0;
    virtual void DrawRoundedRectangle(const Rect& rect, float radiusX, float radiusY, 
                                       Brush* brush, float strokeWidth = 1.0f) = 0;
    virtual void FillRoundedRectangle(const Rect& rect, float radiusX, float radiusY, Brush* brush) = 0;
    virtual void DrawEllipse(const Point& center, float radiusX, float radiusY, 
                              Brush* brush, float strokeWidth = 1.0f) = 0;
    virtual void FillEllipse(const Point& center, float radiusX, float radiusY, Brush* brush) = 0;
    virtual void DrawPath(const Path& path, Brush* brush, float strokeWidth = 1.0f) = 0;
    virtual void FillPath(const Path& path, Brush* brush) = 0;
    virtual void DrawText(const std::wstring& text, const TextFormat& format, 
                          const Rect& rect, Brush* brush) = 0;
    virtual void DrawImage(Image* image, const Rect& destRect, 
                           const Rect& srcRect, float opacity = 1.0f) = 0;
    
    // 获取原始D2D设备上下文（高级用法）
    virtual ID2D1DeviceContext* GetD2DContext() = 0;
};

// ==================== 特效 ====================

class IEffect : public RefCounted {
public:
    virtual ~IEffect() = default;
    virtual void Apply(IRenderTarget* target, const Rect& bounds) = 0;
};

class BlurEffect : public IEffect {
public:
    explicit BlurEffect(float radius);
    void SetRadius(float radius);
    void Apply(IRenderTarget* target, const Rect& bounds) override;
private:
    float m_radius;
};

class DropShadowEffect : public IEffect {
public:
    DropShadowEffect(const Vector2& offset, float blurRadius, const Color& color);
    void SetOffset(const Vector2& offset);
    void SetBlurRadius(float radius);
    void SetColor(const Color& color);
    void Apply(IRenderTarget* target, const Rect& bounds) override;
private:
    Vector2 m_offset;
    float m_blurRadius;
    Color m_color;
};

class ColorMatrixEffect : public IEffect {
public:
    // 5x4 matrix for RGBA transform
    void SetMatrix(const float matrix[5][4]);
    void Apply(IRenderTarget* target, const Rect& bounds) override;
private:
    float m_matrix[5][4];
};

// ==================== 渲染引擎 ====================

class RenderEngine : public IRenderTarget {
public:
    RenderEngine();
    ~RenderEngine();
    
    // 初始化和销毁
    bool Initialize(HWND hwnd);
    void Shutdown();
    bool IsInitialized() const;
    
    // 帧控制
    void BeginFrame();
    void EndFrame();
    void Present();
    bool CheckResize();
    
    // 性能统计
    struct FrameStats {
        float deltaTime;
        float fps;
        int drawCallCount;
        int triangleCount;
    };
    FrameStats GetStats() const;
    
    // 设置目标帧率
    void SetTargetFrameRate(int fps);
    int GetTargetFrameRate() const;
    
    // 窗口效果（Windows 10+）
    void EnableAcrylicEffect(const Color& tintColor);
    void EnableMicaEffect();
    void SetBackdropType(BackdropType type);
    
    // IRenderTarget 实现
    void PushTransform(const Transform& transform) override;
    void PopTransform() override;
    void PushClip(const Rect& rect) override;
    void PushClip(const Path& path) override;
    void PopClip() override;
    void PushOpacity(float opacity) override;
    void PopOpacity() override;
    
    void Clear(const Color& color) override;
    void DrawLine(const Point& p1, const Point& p2, Brush* brush, float strokeWidth) override;
    void DrawRectangle(const Rect& rect, Brush* brush, float strokeWidth) override;
    void FillRectangle(const Rect& rect, Brush* brush) override;
    void DrawRoundedRectangle(const Rect& rect, float radiusX, float radiusY, 
                               Brush* brush, float strokeWidth) override;
    void FillRoundedRectangle(const Rect& rect, float radiusX, float radiusY, Brush* brush) override;
    void DrawEllipse(const Point& center, float radiusX, float radiusY, 
                      Brush* brush, float strokeWidth) override;
    void FillEllipse(const Point& center, float radiusX, float radiusY, Brush* brush) override;
    void DrawPath(const Path& path, Brush* brush, float strokeWidth) override;
    void FillPath(const Path& path, Brush* brush) override;
    void DrawText(const std::wstring& text, const TextFormat& format, 
                  const Rect& rect, Brush* brush) override;
    void DrawImage(Image* image, const Rect& destRect, 
                   const Rect& srcRect, float opacity) override;
    
    ID2D1DeviceContext* GetD2DContext() override;
    
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ==================== 字体管理器 ====================

class FontManager {
public:
    static FontManager& GetInstance();
    
    // 字体加载
    bool LoadFontFromFile(const std::wstring& path);
    bool LoadFontFromMemory(const void* data, size_t size);
    
    // 获取字体集合
    IDWriteFontCollection* GetFontCollection();
    
    // 获取默认字体
    std::wstring GetDefaultFontFamily() const;
    void SetDefaultFontFamily(const std::wstring& family);
    
    // 文本测量
    Size MeasureText(const std::wstring& text, const TextFormat& format, 
                     float maxWidth = FLT_MAX);
    
private:
    FontManager();
    ~FontManager();
    
    ComPtr<IDWriteFactory> m_writeFactory;
    ComPtr<IDWriteFontCollection> m_fontCollection;
    std::wstring m_defaultFontFamily;
    std::vector<ComPtr<IDWriteFontFile>> m_loadedFonts;
};

// ==================== 渲染优化 ====================

class RenderOptimizer {
public:
    // 脏矩形管理
    void InvalidateRect(const Rect& rect);
    void InvalidateAll();
    bool IsRectDirty(const Rect& rect) const;
    const std::vector<Rect>& GetDirtyRects() const;
    void ClearDirtyRects();
    
    // 缓存管理
    void EnableCaching(UIElement* element);
    void InvalidateCache(UIElement* element);
    void RenderToCache(UIElement* element, IRenderTarget* target);
    
    // 绘制调用批处理
    void BeginBatch();
    void EndBatch(IRenderTarget* target);
    void AddDrawCommand(const DrawCommand& cmd);
    
    // 自适应质量
    void SetQualityLevel(QualityLevel level);
    QualityLevel GetQualityLevel() const;
    void AdaptiveQuality(float frameTime);
    
private:
    std::vector<Rect> m_dirtyRects;
    std::unordered_map<UIElement*, CachedBitmap> m_cacheMap;
    std::vector<DrawCommand> m_batchQueue;
    QualityLevel m_qualityLevel;
};

}} // namespace LuaUI::Render
```

## 5. 渲染流程详解

```cpp
// 典型渲染流程
void App::RunRenderLoop() {
    RenderEngine engine;
    engine.Initialize(m_hwnd);
    engine.SetTargetFrameRate(60);
    
    MSG msg;
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (m_running) {
        // 处理Windows消息
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // 计算DeltaTime
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        
        // 更新动画
        AnimationManager::GetInstance().Update(deltaTime);
        
        // 布局更新
        if (m_layoutDirty) {
            LayoutEngine::GetInstance().ProcessLayoutQueue();
        }
        
        // 开始渲染帧
        engine.BeginFrame();
        
        // 清空背景
        engine.Clear(Color::FromHex(0xFFF3F3F3));
        
        // 渲染视觉树
        RenderVisualTree(engine, m_rootElement);
        
        // 结束渲染并显示
        engine.EndFrame();
        engine.Present();
        
        // 帧率控制
        FrameRateControl(engine.GetTargetFrameRate());
    }
    
    engine.Shutdown();
}
```

## 6. 性能优化策略

### 6.1 脏矩形渲染

```cpp
void RenderEngine::RenderElement(UIElement* element) {
    Rect bounds = element->GetGlobalBounds();
    
    // 检查是否需要重绘
    if (!m_optimizer.IsRectDirty(bounds) && element->HasValidCache()) {
        // 使用缓存
        DrawCachedElement(element);
        return;
    }
    
    // 创建裁剪区域
    PushClip(bounds);
    
    // 渲染背景
    if (element->GetBackground()) {
        FillRectangle(bounds, element->GetBackground());
    }
    
    // 渲染内容
    element->OnRender(this);
    
    // 渲染子元素
    for (auto child : element->GetChildren()) {
        if (child->IsVisible() && bounds.Intersects(child->GetGlobalBounds())) {
            RenderElement(child);
        }
    }
    
    // 渲染前景/边框
    if (element->GetBorderBrush()) {
        DrawRectangle(bounds, element->GetBorderBrush(), element->GetBorderThickness());
    }
    
    PopClip();
}
```

### 6.2 离屏缓存

```cpp
class CachedBitmap {
public:
    void Create(int width, int height, ID2D1DeviceContext* context);
    void Update(const std::function<void(IRenderTarget*)>& renderFunc);
    void Draw(IRenderTarget* target, const Point& position, float opacity = 1.0f);
    void Invalidate();
    bool IsValid() const;
    
private:
    ComPtr<ID2D1Bitmap> m_bitmap;
    int m_width, m_height;
    bool m_valid;
};

// 使用场景：复杂控件缓存
void ComplexControl::OnRender(IRenderTarget* target) {
    if (!m_cache.IsValid()) {
        m_cache.Update([this](IRenderTarget* cache) {
            // 渲染复杂内容到缓存
            RenderComplexContent(cache);
        });
    }
    
    // 绘制缓存
    m_cache.Draw(target, GetPosition(), GetOpacity());
}
```

### 6.3 绘制调用合并

```cpp
void RenderOptimizer::AddDrawCommand(const DrawCommand& cmd) {
    // 尝试合并相同材质的绘制调用
    if (!m_batchQueue.empty() && CanMerge(m_batchQueue.back(), cmd)) {
        m_batchQueue.back().rect = MergeRects(m_batchQueue.back().rect, cmd.rect);
    } else {
        m_batchQueue.push_back(cmd);
    }
}
```

## 7. 视觉效果实现

### 7.1 亚克力效果 (Windows 10+)

```cpp
void RenderEngine::EnableAcrylicEffect(const Color& tintColor) {
    // 使用 DirectComposition 创建亚克力背景
    if (m_compositionDevice) {
        // 创建模糊效果
        ComPtr<IDCompositionGaussianBlurEffect> blurEffect;
        m_compositionDevice->CreateGaussianBlurEffect(&blurEffect);
        blurEffect->SetStandardDeviation(30.0f);
        blurEffect->SetBorderMode(D2D1_BORDER_MODE_HARD);
        
        // 创建饱和度效果
        ComPtr<IDCompositionSaturationEffect> saturationEffect;
        m_compositionDevice->CreateSaturationEffect(&saturationEffect);
        saturationEffect->SetSaturation(1.5f);
        
        // 应用效果链
        saturationEffect->SetInput(0, blurEffect.Get());
        
        // 设置色调叠加
        m_compositionVisual->SetEffect(saturationEffect.Get());
        m_compositionVisual->SetTintColor(tintColor.ToD2D());
        m_compositionVisual->SetTintOpacity(0.6f);
    }
}
```

### 7.2 阴影效果

```cpp
void RenderEngine::DrawShadow(const Rect& rect, float blurRadius, 
                               const Vector2& offset, const Color& color) {
    // 创建阴影图层
    ComPtr<ID2D1Effect> shadowEffect;
    m_d2dContext->CreateEffect(CLSID_D2D1Shadow, &shadowEffect);
    
    // 设置模糊
    shadowEffect->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, blurRadius);
    shadowEffect->SetValue(D2D1_SHADOW_PROP_COLOR, color.ToD2D());
    
    // 绘制阴影
    m_d2dContext->DrawImage(shadowEffect.Get(), 
                            D2D1::Point2F(rect.x + offset.x, rect.y + offset.y));
}
```

## 8. 字体渲染优化

```cpp
class TextRenderer {
public:
    // 字形缓存
    struct GlyphCache {
        ComPtr<ID2D1Bitmap> bitmap;
        int advanceWidth;
        int leftSideBearing;
    };
    
    // 文本布局缓存
    void LayoutText(const std::wstring& text, const TextFormat& format, 
                    float maxWidth, TextLayout& outLayout);
    
    // 快速文本渲染（使用缓存）
    void DrawTextFast(const std::wstring& text, const TextFormat& format,
                      const Rect& rect, Brush* brush);
    
private:
    std::unordered_map<uint64_t, GlyphCache> m_glyphCache;
    std::unordered_map<std::wstring, ComPtr<IDWriteTextLayout>> m_layoutCache;
};
```

## 9. 多线程渲染考虑

```cpp
class ThreadedRenderer {
public:
    // 在后台线程构建绘制命令
    void BuildDrawCommandsAsync(UIElement* root);
    
    // 主线程执行绘制
    void ExecuteDrawCommands(IRenderTarget* target);
    
private:
    std::thread m_workerThread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::vector<DrawCommand> m_pendingCommands;
    bool m_commandsReady;
};
```

## 10. 调试与性能分析

```cpp
class RenderDebugger {
public:
    // 显示脏矩形
    void ShowDirtyRects(bool enable);
    
    // 显示绘制调用
    void ShowDrawCalls(bool enable);
    
    // 显示帧时间
    void ShowFrameTime(bool enable);
    
    // 捕获帧
    void CaptureFrame(const std::string& filename);
    
    // 性能分析
    void BeginProfile(const char* name);
    void EndProfile(const char* name);
    void DumpProfileData();
};
```
