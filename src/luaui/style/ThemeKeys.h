#pragma once

namespace luaui {
namespace theme {

// ============================================================================
// 全局通用
// ============================================================================
inline constexpr const char* kAccentColor       = "AccentColor";
inline constexpr const char* kBackgroundPrimary  = "BackgroundPrimary";
inline constexpr const char* kBackgroundSecondary= "BackgroundSecondary";
inline constexpr const char* kTextPrimary        = "TextPrimary";
inline constexpr const char* kTextSecondary      = "TextSecondary";
inline constexpr const char* kTextDisabled       = "TextDisabled";
inline constexpr const char* kBorderNormal       = "BorderNormal";
inline constexpr const char* kBorderFocused      = "BorderFocused";
inline constexpr const char* kFocusVisual        = "FocusVisual";

// ============================================================================
// Button
// ============================================================================
inline constexpr const char* kButtonNormalBg     = "ButtonNormalBg";
inline constexpr const char* kButtonHoverBg      = "ButtonHoverBg";
inline constexpr const char* kButtonPressedBg    = "ButtonPressedBg";
inline constexpr const char* kButtonDisabledBg   = "ButtonDisabledBg";
inline constexpr const char* kButtonDisabledFg   = "ButtonDisabledFg";
inline constexpr const char* kButtonDisabledBorder= "ButtonDisabledBorder";
inline constexpr const char* kButtonBorder       = "ButtonBorder";
inline constexpr const char* kButtonForeground   = "ButtonForeground";
inline constexpr const char* kButtonNormalBorder = "ButtonNormalBorder";
inline constexpr const char* kButtonHoverBorder  = "ButtonHoverBorder";
inline constexpr const char* kButtonPressedBorder= "ButtonPressedBorder";

// ============================================================================
// CheckBox / RadioButton
// ============================================================================
inline constexpr const char* kCheckNormalBorder  = "CheckNormalBorder";
inline constexpr const char* kCheckHoverBorder   = "CheckHoverBorder";
inline constexpr const char* kCheckPressedBorder = "CheckPressedBorder";
inline constexpr const char* kCheckMark          = "CheckMark";
inline constexpr const char* kCheckBackground    = "CheckBackground";
inline constexpr const char* kCheckDisabledBorder= "CheckDisabledBorder";
inline constexpr const char* kCheckDisabledMark  = "CheckDisabledMark";
inline constexpr const char* kCheckDisabledText  = "CheckDisabledText";

// ============================================================================
// Slider
// ============================================================================
inline constexpr const char* kSliderTrack        = "SliderTrack";
inline constexpr const char* kSliderProgress     = "SliderProgress";
inline constexpr const char* kSliderThumbBg      = "SliderThumbBg";
inline constexpr const char* kSliderThumbBorder  = "SliderThumbBorder";
inline constexpr const char* kSliderThumb        = "SliderThumb";

// ============================================================================
// TextBox
// ============================================================================
inline constexpr const char* kTextBoxBackground  = "TextBoxBackground";
inline constexpr const char* kTextBoxBorder      = "TextBoxBorder";
inline constexpr const char* kTextBoxFocusedBorder= "TextBoxFocusedBorder";
inline constexpr const char* kTextBoxSelection   = "TextBoxSelection";
inline constexpr const char* kTextBoxInactiveSel = "TextBoxInactiveSelection";
inline constexpr const char* kTextBoxPlaceholder = "TextBoxPlaceholder";
inline constexpr const char* kTextBoxReadOnlyBg  = "TextBoxReadOnlyBg";

// ============================================================================
// ProgressBar
// ============================================================================
inline constexpr const char* kProgressBackground = "ProgressBackground";
inline constexpr const char* kProgressForeground = "ProgressForeground";
inline constexpr const char* kProgressBorder     = "ProgressBorder";

// ============================================================================
// TabControl
// ============================================================================
inline constexpr const char* kTabStripBg         = "TabStripBg";
inline constexpr const char* kTabSelectedBg      = "TabSelectedBg";
inline constexpr const char* kTabHoverBg         = "TabHoverBg";
inline constexpr const char* kTabBorder          = "TabBorder";

// ============================================================================
// ScrollBar / ScrollViewer
// ============================================================================
inline constexpr const char* kScrollBarTrack     = "ScrollBarTrack";
inline constexpr const char* kScrollBarThumb     = "ScrollBarThumb";
inline constexpr const char* kScrollBarThumbHover= "ScrollBarThumbHover";

} // namespace theme
} // namespace luaui
