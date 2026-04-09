#include "Theme.h"
#include "ThemeKeys.h"

namespace luaui {
namespace controls {

Theme& Theme::GetCurrent() {
    static Theme instance;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        instance.ApplyResources(CreateLightTheme());
    }
    return instance;
}

/** @brief 创建 Light 主题资源 */
ResourceDictionary CreateLightTheme() {
    using namespace rendering;
    using namespace theme;
    ResourceDictionary t;

    // 按钮颜色
    t.AddColor(kButtonNormalBg, Color::FromHex(0xE0E0E0));
    t.AddColor(kButtonHoverBg, Color::FromHex(0xC0C0C0));
    t.AddColor(kButtonPressedBg, Color::FromHex(0xA0A0A0));
    t.AddColor(kButtonNormalBorder, Color::FromHex(0x808080));
    t.AddColor(kButtonHoverBorder, Color::FromHex(0x606060));
    t.AddColor(kButtonPressedBorder, Color::FromHex(0x404040));
    t.AddColor(kButtonForeground, Color::FromHex(0x000000));

    // 滑块颜色
    t.AddColor(kSliderTrack, Color::FromHex(0xE0E0E0));
    t.AddColor(kSliderProgress, Color::FromHex(0x0078D7));
    t.AddColor(kSliderThumb, Color::FromHex(0x0078D7));

    // 复选框颜色
    t.AddColor(kCheckNormalBorder, Color::FromHex(0x808080));
    t.AddColor(kCheckHoverBorder, Color::FromHex(0x0078D7));
    t.AddColor(kCheckMark, Color::FromHex(0x0078D7));

    // TextBox 颜色
    t.AddColor(kTextBoxBackground, Color::FromHex(0xFFFFFF));
    t.AddColor(kTextBoxBorder, Color::FromHex(0x808080));
    t.AddColor(kTextBoxFocusedBorder, Color::FromHex(0x0078D7));
    t.AddColor(kTextBoxSelection, Color::FromHex(0x0078D7));
    t.AddColor(kTextBoxInactiveSel, Color::FromHex(0xD8D8D8));
    t.AddColor(kTextBoxPlaceholder, Color::FromHex(0x8A8A8A));
    t.AddColor(kTextBoxReadOnlyBg, Color::FromHex(0xF5F5F5));

    // ProgressBar 颜色
    t.AddColor(kProgressBackground, Color::FromHex(0xE0E0E0));
    t.AddColor(kProgressForeground, Color::FromHex(0x0078D7));
    t.AddColor(kProgressBorder, Color::FromHex(0xCCCCCC));

    return t;
}

/** @brief 创建 Dark 主题资源 */
ResourceDictionary CreateDarkTheme() {
    using namespace rendering;
    using namespace theme;
    ResourceDictionary t;

    // 按钮颜色
    t.AddColor(kButtonNormalBg, Color::FromHex(0x2D2D2D));
    t.AddColor(kButtonHoverBg, Color::FromHex(0x404040));
    t.AddColor(kButtonPressedBg, Color::FromHex(0x505050));
    t.AddColor(kButtonNormalBorder, Color::FromHex(0x555555));
    t.AddColor(kButtonHoverBorder, Color::FromHex(0x0078D7));
    t.AddColor(kButtonPressedBorder, Color::FromHex(0x0078D7));
    t.AddColor(kButtonForeground, Color::FromHex(0xFFFFFF));

    // 滑块颜色
    t.AddColor(kSliderTrack, Color::FromHex(0x3D3D3D));
    t.AddColor(kSliderProgress, Color::FromHex(0x0078D7));
    t.AddColor(kSliderThumb, Color::FromHex(0x0078D7));

    // 复选框颜色
    t.AddColor(kCheckNormalBorder, Color::FromHex(0x555555));
    t.AddColor(kCheckHoverBorder, Color::FromHex(0x0078D7));
    t.AddColor(kCheckMark, Color::FromHex(0x0078D7));

    // TextBox 颜色
    t.AddColor(kTextBoxBackground, Color::FromHex(0x2D2D2D));
    t.AddColor(kTextBoxBorder, Color::FromHex(0x555555));
    t.AddColor(kTextBoxFocusedBorder, Color::FromHex(0x0078D7));
    t.AddColor(kTextBoxSelection, Color::FromHex(0x0078D7));
    t.AddColor(kTextBoxInactiveSel, Color::FromHex(0x3D3D3D));
    t.AddColor(kTextBoxPlaceholder, Color::FromHex(0x8A8A8A));
    t.AddColor(kTextBoxReadOnlyBg, Color::FromHex(0x252525));

    // ProgressBar 颜色
    t.AddColor(kProgressBackground, Color::FromHex(0x3D3D3D));
    t.AddColor(kProgressForeground, Color::FromHex(0x0078D7));
    t.AddColor(kProgressBorder, Color::FromHex(0x555555));

    return t;
}

} // namespace controls
} // namespace luaui
