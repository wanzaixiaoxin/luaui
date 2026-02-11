#pragma once

#include "Control.h"
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
 * 注意：这是一个框架实现，实际图像渲染需要渲染层支持IBitmap接口
 */
class Image : public Control {
public:
    Image();
    
    std::string GetTypeName() const override { return "Image"; }
    
    // 拉伸模式
    Stretch GetStretch() const { return m_stretch; }
    void SetStretch(Stretch stretch);
    
    // 透明度
    float GetOpacity() const { return m_opacity; }
    void SetOpacity(float opacity);
    
    // 图像尺寸（模拟）
    float GetNaturalWidth() const { return m_naturalWidth; }
    float GetNaturalHeight() const { return m_naturalHeight; }
    
    // 设置图像源（简化版，使用路径）
    void SetSourcePath(const std::wstring& path) { m_sourcePath = path; }
    std::wstring GetSourcePath() const { return m_sourcePath; }

protected:
    void InitializeComponents() override;

private:
    std::wstring m_sourcePath;
    Stretch m_stretch = Stretch::Uniform;
    float m_opacity = 1.0f;
    float m_naturalWidth = 0.0f;
    float m_naturalHeight = 0.0f;
    bool m_isLoaded = false;
};

} // namespace controls
} // namespace luaui
