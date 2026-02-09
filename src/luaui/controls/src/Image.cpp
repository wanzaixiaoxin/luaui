#include "Image.h"
#include "IRenderContext.h"
#include <algorithm>

namespace luaui {
namespace controls {

// 内部位图资源封装
class Image::BitmapResource {
public:
    // 这里应该存储实际的位图资�?
    // 简化实现：只存储尺寸信�?
    float Width = 0;
    float Height = 0;
    bool Loaded = false;
};

Image::Image() {
    m_bitmap = std::make_shared<BitmapResource>();
}

void Image::SetSource(const std::wstring& path) {
    if (m_source == path) return;
    
    m_source = path;
    ReleaseBitmap();
    
    // 尝试加载位图
    if (LoadBitmap()) {
        InvalidateMeasure();
    }
}

void Image::SetSourceFromMemory(const void* /*data*/, size_t /*size*/) {
    // TODO: 从内存加载图�?
    ReleaseBitmap();
    InvalidateMeasure();
}

void Image::SetStretch(Stretch stretch) {
    if (m_stretch == stretch) return;
    
    m_stretch = stretch;
    InvalidateMeasure();
}

bool Image::LoadBitmap() {
    // TODO: 使用 WIC 加载实际图片
    // 简化实现：模拟加载成功，设置默认尺�?
    
    if (m_source.empty()) {
        m_bitmap->Loaded = false;
        return false;
    }
    
    // 模拟加载：假设图片是 100x100（实际应该从文件加载�?
    m_bitmap->Width = 100;
    m_bitmap->Height = 100;
    m_bitmap->Loaded = true;
    
    m_imageWidth = m_bitmap->Width;
    m_imageHeight = m_bitmap->Height;
    
    return true;
}

void Image::ReleaseBitmap() {
    if (m_bitmap) {
        m_bitmap->Loaded = false;
        m_bitmap->Width = 0;
        m_bitmap->Height = 0;
    }
    m_imageWidth = 0;
    m_imageHeight = 0;
}

Size Image::MeasureOverride(const Size& availableSize) {
    if (!m_bitmap || !m_bitmap->Loaded) {
        // 没有图片时返�?0 或固定尺�?
        return Size(0, 0);
    }
    
    float imgWidth = m_imageWidth;
    float imgHeight = m_imageHeight;
    
    switch (m_stretch) {
        case Stretch::None:
            // 保持原始尺寸
            return Size(imgWidth, imgHeight);
            
        case Stretch::Fill:
            // 填充可用区域
            return Size(availableSize.width, availableSize.height);
            
        case Stretch::Uniform: {
            // 等比缩放，完整显�?
            float scaleX = availableSize.width / imgWidth;
            float scaleY = availableSize.height / imgHeight;
            float scale = std::min(scaleX, scaleY);
            
            if (scale == std::numeric_limits<float>::max() || scale <= 0) {
                scale = 1.0f;
            }
            
            return Size(imgWidth * scale, imgHeight * scale);
        }
            
        case Stretch::UniformToFill: {
            // 等比缩放，填满区域（可能裁剪�?
            float scaleX = availableSize.width / imgWidth;
            float scaleY = availableSize.height / imgHeight;
            float scale = std::max(scaleX, scaleY);
            
            if (scale == std::numeric_limits<float>::max() || scale <= 0) {
                scale = 1.0f;
            }
            
            return Size(imgWidth * scale, imgHeight * scale);
        }
    }
    
    return Size(imgWidth, imgHeight);
}

void Image::Render(IRenderContext* context) {
    if (!m_bitmap || !m_bitmap->Loaded) {
        // 绘制占位�?
        auto brush = context->CreateSolidColorBrush(Color(0.9f, 0.9f, 0.9f, 1.0f));
        context->FillRectangle(m_renderRect, brush.get());
        
        auto borderBrush = context->CreateSolidColorBrush(Color(0.7f, 0.7f, 0.7f, 1.0f));
        context->DrawRectangle(m_renderRect, borderBrush.get(), 1.0f);
        return;
    }
    
    // TODO: 实际渲染位图
    // 简化实现：绘制一个带颜色的矩形表示图�?
    auto brush = context->CreateSolidColorBrush(Color(0.8f, 0.8f, 0.9f, 1.0f));
    context->FillRectangle(m_renderRect, brush.get());
}

} // namespace controls
} // namespace luaui
