#pragma once

#include "Control.h"
#include "../core/Components/LayoutComponent.h"
#include "../core/Components/RenderComponent.h"
#include "../rendering/Types.h"
#include "../rendering/IBitmap.h"
#include <string>

namespace luaui {
namespace controls {

/**
 * @brief 拉伸模式
 */
enum class Stretch {
    None,       // 不拉伸，保持原始尺寸
    Fill,       // 填充整个区域，可能变形
    Uniform,    // 等比缩放，保持比例，可能有空白
    UniformToFill  // 等比缩放，填满区域，可能裁剪
};

/**
 * @brief Image 图像控件（新架构）
 * 
 * 支持从文件加载并渲染图像
 */
class Image : public luaui::Control {
public:
    Image();
    
    std::string GetTypeName() const override { return "Image"; }
    
    // 拉伸模式
    Stretch GetStretch() const { return m_stretch; }
    void SetStretch(Stretch stretch);
    
    // 透明度
    float GetOpacity() const { return m_opacity; }
    void SetOpacity(float opacity);
    
    // 图像尺寸
    float GetNaturalWidth() const { return m_naturalWidth; }
    float GetNaturalHeight() const { return m_naturalHeight; }
    
    // 设置图像源
    void SetSourcePath(const std::wstring& path);
    std::wstring GetSourcePath() const { return m_sourcePath; }
    
    // 加载图像
    bool LoadFromFile(const std::wstring& filePath);
    void Unload();
    bool IsLoaded() const { return m_isLoaded && m_bitmap != nullptr; }

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;

private:
    void DrawPlaceholder(rendering::IRenderContext* context, const rendering::Rect& rect);
    void DrawBitmap(rendering::IRenderContext* context, const rendering::Rect& rect);
    void UpdateNaturalSize();
    
    std::wstring m_sourcePath;
    Stretch m_stretch = Stretch::Uniform;
    float m_opacity = 1.0f;
    float m_naturalWidth = 0.0f;
    float m_naturalHeight = 0.0f;
    bool m_isLoaded = false;
    bool m_loadFailed = false;
    
    // 缓存的位图
    rendering::IBitmapPtr m_bitmap;
    
    // 占位符颜色
    rendering::Color m_placeholderColor = rendering::Color::FromHex(0xE0E0E0);
    rendering::Color m_borderColor = rendering::Color::FromHex(0xAAAAAA);
};

} // namespace controls
} // namespace luaui
