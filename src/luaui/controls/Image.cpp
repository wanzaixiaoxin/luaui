#include "Image.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "IRenderContext.h"
#include "Logger.h"
#include <algorithm>

namespace luaui {
namespace controls {

Image::Image() {}

void Image::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    
    if (auto* layout = GetLayout()) {
        layout->SetMinWidth(16);
        layout->SetMinHeight(16);
    }
}

void Image::SetStretch(Stretch stretch) {
    if (m_stretch != stretch) {
        m_stretch = stretch;
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Image::SetOpacity(float opacity) {
    opacity = std::max(0.0f, std::min(1.0f, opacity));
    if (m_opacity != opacity) {
        m_opacity = opacity;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Image::SetSourcePath(const std::wstring& path) {
    if (m_sourcePath != path) {
        m_sourcePath = path;
        // 路径改变时重新加载
        if (!path.empty()) {
            LoadFromFile(path);
        } else {
            Unload();
        }
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

bool Image::LoadFromFile(const std::wstring& filePath) {
    // 延迟加载：实际加载在渲染时进行，当 RenderContext 可用时
    m_sourcePath = filePath;
    m_isLoaded = false;
    m_loadFailed = false;
    m_bitmap.reset();
    
    utils::Logger::InfoF("[Image] Set source path: %s", 
        std::string(filePath.begin(), filePath.end()).c_str());
    
    return true;
}

void Image::Unload() {
    m_bitmap.reset();
    m_isLoaded = false;
    m_loadFailed = false;
    m_naturalWidth = 0;
    m_naturalHeight = 0;
}

void Image::UpdateNaturalSize() {
    if (m_bitmap) {
        m_naturalWidth = static_cast<float>(m_bitmap->GetWidth());
        m_naturalHeight = static_cast<float>(m_bitmap->GetHeight());
    } else {
        m_naturalWidth = 0;
        m_naturalHeight = 0;
    }
}

rendering::Size Image::OnMeasure(const rendering::Size& availableSize) {
    // 如果有自然尺寸，根据 Stretch 模式计算
    if (m_naturalWidth > 0 && m_naturalHeight > 0) {
        switch (m_stretch) {
            case Stretch::None:
                return rendering::Size(m_naturalWidth, m_naturalHeight);
            case Stretch::Fill:
                return availableSize;
            case Stretch::Uniform: {
                float scale = std::min(
                    availableSize.width / m_naturalWidth,
                    availableSize.height / m_naturalHeight
                );
                return rendering::Size(m_naturalWidth * scale, m_naturalHeight * scale);
            }
            case Stretch::UniformToFill: {
                float scale = std::max(
                    availableSize.width / m_naturalWidth,
                    availableSize.height / m_naturalHeight
                );
                return rendering::Size(m_naturalWidth * scale, m_naturalHeight * scale);
            }
        }
    }
    
    // 默认返回可用空间或固定大小
    if (availableSize.width > 0 && availableSize.height > 0) {
        return availableSize;
    }
    return rendering::Size(100, 100);  // 默认大小
}

void Image::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 尝试加载位图（如果尚未加载）
    if (!m_isLoaded && !m_loadFailed && !m_sourcePath.empty()) {
        m_bitmap = context->LoadBitmapFromFile(m_sourcePath);
        if (m_bitmap) {
            m_isLoaded = true;
            UpdateNaturalSize();
            // 加载成功后重新测量布局
            if (auto* layout = GetLayout()) {
                layout->InvalidateMeasure();
            }
            utils::Logger::InfoF("[Image] Loaded successfully: %dx%d", 
                m_bitmap->GetWidth(), m_bitmap->GetHeight());
        } else {
            m_loadFailed = true;
            utils::Logger::WarningF("[Image] Failed to load: %s", 
                std::string(m_sourcePath.begin(), m_sourcePath.end()).c_str());
        }
    }
    
    // 如果有位图则绘制，否则绘制占位符
    if (m_bitmap && m_isLoaded) {
        DrawBitmap(context, rect);
    } else {
        DrawPlaceholder(context, rect);
    }
}

void Image::DrawBitmap(rendering::IRenderContext* context, const rendering::Rect& rect) {
    if (!m_bitmap) return;
    
    rendering::Rect destRect = rect;
    
    // 根据 Stretch 模式计算目标矩形
    if (m_naturalWidth > 0 && m_naturalHeight > 0) {
        switch (m_stretch) {
            case Stretch::None:
                // 保持原始尺寸，居中显示
                destRect = rendering::Rect(
                    rect.x + (rect.width - m_naturalWidth) / 2,
                    rect.y + (rect.height - m_naturalHeight) / 2,
                    m_naturalWidth,
                    m_naturalHeight
                );
                break;
                
            case Stretch::Fill:
                // 填充整个区域（可能变形）
                destRect = rect;
                break;
                
            case Stretch::Uniform: {
                // 等比缩放，保持比例，可能有空白
                float scale = std::min(
                    rect.width / m_naturalWidth,
                    rect.height / m_naturalHeight
                );
                float w = m_naturalWidth * scale;
                float h = m_naturalHeight * scale;
                destRect = rendering::Rect(
                    rect.x + (rect.width - w) / 2,
                    rect.y + (rect.height - h) / 2,
                    w, h
                );
                break;
            }
            
            case Stretch::UniformToFill: {
                // 等比缩放，填满区域，可能裁剪
                float scale = std::max(
                    rect.width / m_naturalWidth,
                    rect.height / m_naturalHeight
                );
                float w = m_naturalWidth * scale;
                float h = m_naturalHeight * scale;
                destRect = rendering::Rect(
                    rect.x + (rect.width - w) / 2,
                    rect.y + (rect.height - h) / 2,
                    w, h
                );
                break;
            }
        }
    }
    
    // 绘制位图（应用透明度）
    context->DrawBitmap(m_bitmap.get(), destRect, m_opacity);
}

void Image::DrawPlaceholder(rendering::IRenderContext* context, const rendering::Rect& rect) {
    // 填充背景
    auto bgBrush = context->CreateSolidColorBrush(m_placeholderColor);
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    
    // 绘制边框
    auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
    if (borderBrush) {
        context->DrawRectangle(rect, borderBrush.get(), 1.0f);
    }
    
    // 绘制对角线（表示图像占位符）
    auto lineBrush = context->CreateSolidColorBrush(rendering::Color::FromHex(0xCCCCCC));
    if (lineBrush) {
        context->DrawLine(
            rendering::Point(rect.x, rect.y),
            rendering::Point(rect.x + rect.width, rect.y + rect.height),
            lineBrush.get(), 1.0f
        );
        context->DrawLine(
            rendering::Point(rect.x + rect.width, rect.y),
            rendering::Point(rect.x, rect.y + rect.height),
            lineBrush.get(), 1.0f
        );
    }
    
    // 绘制文件名（如果有）
    if (!m_sourcePath.empty() && !m_loadFailed) {
        auto textBrush = context->CreateSolidColorBrush(rendering::Color::FromHex(0x666666));
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", 10);
        if (textBrush && textFormat) {
            std::wstring filename = m_sourcePath;
            size_t pos = filename.find_last_of(L"/\\");
            if (pos != std::wstring::npos) {
                filename = filename.substr(pos + 1);
            }
            rendering::Point textPos(rect.x + 4, rect.y + 4);
            context->DrawTextString(filename, textFormat.get(), textPos, textBrush.get());
        }
    }
}

} // namespace controls
} // namespace luaui
