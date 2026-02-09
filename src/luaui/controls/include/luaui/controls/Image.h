#pragma once

#include "Control.h"
#include <string>

namespace luaui {
namespace controls {

// 图片拉伸模式
enum class Stretch {
    None,       // 不拉伸，保持原始尺寸
    Fill,       // 填充整个区域（可能变形）
    Uniform,    // 等比缩放，完整显示（可能有黑边）
    UniformToFill  // 等比缩放，填满区域（可能裁剪）
};

// ==================== Image 控件 ====================
class Image : public Control {
public:
    Image();
    
    std::string GetTypeName() const override { return "Image"; }

    // 图片来源
    std::wstring GetSource() const { return m_source; }
    void SetSource(const std::wstring& path);
    
    // 从内存加载
    void SetSourceFromMemory(const void* data, size_t size);
    
    // 拉伸模式
    Stretch GetStretch() const { return m_stretch; }
    void SetStretch(Stretch stretch);

    // 图片尺寸（只读）
    float GetImageWidth() const { return m_imageWidth; }
    float GetImageHeight() const { return m_imageHeight; }

    void Render(IRenderContext* context) override;

protected:
    Size MeasureOverride(const Size& availableSize) override;

private:
    std::wstring m_source;
    Stretch m_stretch = Stretch::Uniform;
    
    // 图片原始尺寸
    float m_imageWidth = 0;
    float m_imageHeight = 0;
    
    // 渲染位图资源（内部使用）
    class BitmapResource;
    std::shared_ptr<BitmapResource> m_bitmap;
    
    bool LoadBitmap();
    void ReleaseBitmap();
};

} // namespace controls
} // namespace luaui
