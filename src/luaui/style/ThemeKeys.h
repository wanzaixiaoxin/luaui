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
inline constexpr const char* kTabContentBg       = "TabContentBg";
inline constexpr const char* kTabItemText        = "TabItemText";
inline constexpr const char* kTabItemSelectedText= "TabItemSelectedText";
inline constexpr const char* kTabItemCloseBtn    = "TabItemCloseBtn";
inline constexpr const char* kTabItemCloseBtnHover = "TabItemCloseBtnHover";

// ============================================================================
// ScrollBar / ScrollViewer
// ============================================================================
inline constexpr const char* kScrollBarTrack     = "ScrollBarTrack";
inline constexpr const char* kScrollBarThumb     = "ScrollBarThumb";
inline constexpr const char* kScrollBarThumbHover= "ScrollBarThumbHover";

// ============================================================================
// Menu / MenuBar / MenuItem
// ============================================================================
inline constexpr const char* kMenuBarBg          = "MenuBarBg";
inline constexpr const char* kMenuBarHoverBg     = "MenuBarHoverBg";
inline constexpr const char* kMenuBarOpenBg      = "MenuBarOpenBg";
inline constexpr const char* kMenuBarText        = "MenuBarText";
inline constexpr const char* kMenuBg             = "MenuBg";
inline constexpr const char* kMenuBorder         = "MenuBorder";
inline constexpr const char* kMenuItemHoverBg    = "MenuItemHoverBg";
inline constexpr const char* kMenuItemText       = "MenuItemText";
inline constexpr const char* kMenuItemDisabledText= "MenuItemDisabledText";
inline constexpr const char* kMenuItemSeparator  = "MenuItemSeparator";
inline constexpr const char* kMenuItemCheckMark  = "MenuItemCheckMark";
inline constexpr const char* kMenuItemArrow      = "MenuItemArrow";

// ============================================================================
// Toolbar / ToolbarItem
// ============================================================================
inline constexpr const char* kToolbarBg          = "ToolbarBg";
inline constexpr const char* kToolbarBorder      = "ToolbarBorder";
inline constexpr const char* kToolbarItemHoverBg = "ToolbarItemHoverBg";
inline constexpr const char* kToolbarItemPressedBg = "ToolbarItemPressedBg";
inline constexpr const char* kToolbarItemCheckedBg = "ToolbarItemCheckedBg";
inline constexpr const char* kToolbarItemText    = "ToolbarItemText";
inline constexpr const char* kToolbarItemDisabledText = "ToolbarItemDisabledText";
inline constexpr const char* kToolbarSeparatorLine= "ToolbarSeparatorLine";

// ============================================================================
// StatusBar
// ============================================================================
inline constexpr const char* kStatusBarBg        = "StatusBarBg";
inline constexpr const char* kStatusBarBorder    = "StatusBarBorder";
inline constexpr const char* kStatusBarGrip      = "StatusBarGrip";
inline constexpr const char* kStatusBarItemText  = "StatusBarItemText";

// ============================================================================
// TreeView
// ============================================================================
inline constexpr const char* kTreeViewItemHoverBg    = "TreeViewItemHoverBg";
inline constexpr const char* kTreeViewItemSelectedBg = "TreeViewItemSelectedBg";
inline constexpr const char* kTreeViewItemText       = "TreeViewItemText";
inline constexpr const char* kTreeViewItemSelectedText= "TreeViewItemSelectedText";
inline constexpr const char* kTreeViewExpandBtn      = "TreeViewExpandBtn";

// ============================================================================
// SideBar
// ============================================================================
inline constexpr const char* kSideBarBg           = "SideBarBg";
inline constexpr const char* kSideBarBorder       = "SideBarBorder";
inline constexpr const char* kSideBarHeaderBg     = "SideBarHeaderBg";
inline constexpr const char* kSideBarHeaderText   = "SideBarHeaderText";
inline constexpr const char* kSideBarCollapseBtn  = "SideBarCollapseBtn";
inline constexpr const char* kSideBarCollapseBtnHover = "SideBarCollapseBtnHover";

// ============================================================================
// Splitter
// ============================================================================
inline constexpr const char* kSplitterBg          = "SplitterBg";
inline constexpr const char* kSplitterHoverBg     = "SplitterHoverBg";
inline constexpr const char* kSplitterActiveBg    = "SplitterActiveBg";
inline constexpr const char* kSplitterGrip        = "SplitterGrip";

} // namespace theme
} // namespace luaui
