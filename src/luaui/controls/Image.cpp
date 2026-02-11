#include "Image.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
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

} // namespace controls
} // namespace luaui
