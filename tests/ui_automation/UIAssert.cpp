// UIAssert.cpp - Implementation of UI assertions

#include "UIAutomationFramework.h"
#include "TestHooks.h"
#include "Control.h"
#include "TextBlock.h"
#include "Slider.h"
#include "Panel.h"
#include "Components/RenderComponent.h"
#include "Components/LayoutComponent.h"
#include <cmath>
#include <sstream>

namespace luaui {
namespace test {
namespace automation {

// Helper function to report assertion failure
static void ReportFailure(const std::string& message) {
    throw luaui::test::AssertionError(message);
}

// ============================================================================
// Visibility Assertions
// ============================================================================
void UIAssert::IsVisible(Control* control, const char* message) {
    if (!control) {
        ReportFailure(message ? message : "Control is null");
    }
    if (!control->GetIsVisible()) {
        ReportFailure(message ? message : "Control is not visible");
    }
}

void UIAssert::IsNotVisible(Control* control, const char* message) {
    if (!control) return;  // Null is considered not visible
    if (control->GetIsVisible()) {
        ReportFailure(message ? message : "Control is visible but should not be");
    }
}

void UIAssert::IsEnabled(Control* control, const char* message) {
    if (!control) {
        ReportFailure(message ? message : "Control is null");
    }
    if (!control->GetIsEnabled()) {
        ReportFailure(message ? message : "Control is disabled");
    }
}

void UIAssert::IsDisabled(Control* control, const char* message) {
    if (!control) {
        ReportFailure(message ? message : "Control is null");
    }
    if (control->GetIsEnabled()) {
        ReportFailure(message ? message : "Control is enabled but should be disabled");
    }
}

// ============================================================================
// Position/Size Assertions
// ============================================================================
void UIAssert::HasPosition(Control* control, float x, float y, float tolerance) {
    if (!control) {
        ReportFailure("Control is null");
    }
    
    auto rect = ControlTestHook::GetRenderRect(control);
    
    if (std::abs(rect.x - x) > tolerance || std::abs(rect.y - y) > tolerance) {
        std::stringstream ss;
        ss << "Expected position (" << x << ", " << y << ") but got ("
           << rect.x << ", " << rect.y << ")";
        ReportFailure(ss.str());
    }
}

void UIAssert::HasSize(Control* control, float width, float height, float tolerance) {
    if (!control) {
        ReportFailure("Control is null");
    }
    
    auto rect = ControlTestHook::GetRenderRect(control);
    
    if (std::abs(rect.width - width) > tolerance || std::abs(rect.height - height) > tolerance) {
        std::stringstream ss;
        ss << "Expected size (" << width << "x" << height << ") but got ("
           << rect.width << "x" << rect.height << ")";
        ReportFailure(ss.str());
    }
}

void UIAssert::IsWithin(Control* control, const luaui::rendering::Rect& bounds) {
    if (!control) {
        ReportFailure("Control is null");
    }
    
    auto rect = ControlTestHook::GetRenderRect(control);
    
    if (rect.x < bounds.x || rect.y < bounds.y ||
        rect.x + rect.width > bounds.x + bounds.width ||
        rect.y + rect.height > bounds.y + bounds.height) {
        ReportFailure("Control is not within expected bounds");
    }
}

// ============================================================================
// Property Assertions
// ============================================================================
void UIAssert::HasText(Control* control, const std::wstring& expected) {
    if (!control) {
        ReportFailure("Control is null");
    }
    
    // Try to get text from TextBlock
    auto* textBlock = dynamic_cast<controls::TextBlock*>(control);
    if (textBlock) {
        if (textBlock->GetText() != expected) {
            std::stringstream ss;
            ss << "Expected text \"";
            // Convert wstring to string for error message
            std::string narrow(expected.begin(), expected.end());
            ss << narrow << "\" but got \"";
            std::string actual(textBlock->GetText().begin(), textBlock->GetText().end());
            ss << actual << "\"";
            ReportFailure(ss.str());
        }
        return;
    }
    
    ReportFailure("Control does not support text assertion");
}

void UIAssert::ContainsText(Control* control, const std::wstring& substring) {
    if (!control) {
        ReportFailure("Control is null");
    }
    
    auto* textBlock = dynamic_cast<controls::TextBlock*>(control);
    if (textBlock) {
        if (textBlock->GetText().find(substring) == std::wstring::npos) {
            ReportFailure("Text does not contain expected substring");
        }
        return;
    }
    
    ReportFailure("Control does not support text assertion");
}

void UIAssert::HasValue(Control* control, double expected, double tolerance) {
    if (!control) {
        ReportFailure("Control is null");
    }
    
    auto* slider = dynamic_cast<controls::Slider*>(control);
    if (slider) {
        if (std::abs(slider->GetValue() - expected) > tolerance) {
            std::stringstream ss;
            ss << "Expected value " << expected << " but got " << slider->GetValue();
            ReportFailure(ss.str());
        }
        return;
    }
    
    ReportFailure("Control does not support value assertion");
}

// ============================================================================
// Layout Assertions
// ============================================================================
void UIAssert::IsAligned(Control* control, Control* reference, const char* alignment) {
    if (!control || !reference) {
        ReportFailure("Control or reference is null");
    }
    
    auto controlRect = ControlTestHook::GetRenderRect(control);
    auto refRect = ControlTestHook::GetRenderRect(reference);
    
    std::string align(alignment);
    bool aligned = false;
    
    if (align == "left") {
        aligned = std::abs(controlRect.x - refRect.x) < 1.0f;
    } else if (align == "right") {
        aligned = std::abs((controlRect.x + controlRect.width) - (refRect.x + refRect.width)) < 1.0f;
    } else if (align == "top") {
        aligned = std::abs(controlRect.y - refRect.y) < 1.0f;
    } else if (align == "bottom") {
        aligned = std::abs((controlRect.y + controlRect.height) - (refRect.y + refRect.height)) < 1.0f;
    } else if (align == "center") {
        float controlCenter = controlRect.x + controlRect.width / 2;
        float refCenter = refRect.x + refRect.width / 2;
        aligned = std::abs(controlCenter - refCenter) < 1.0f;
    }
    
    if (!aligned) {
        std::stringstream ss;
        ss << "Controls are not " << alignment << " aligned";
        ReportFailure(ss.str());
    }
}

void UIAssert::IsContainedIn(Control* child, Control* parent) {
    if (!child || !parent) {
        ReportFailure("Child or parent is null");
    }
    
    auto childRect = ControlTestHook::GetRenderRect(child);
    auto parentRect = ControlTestHook::GetRenderRect(parent);
    
    if (childRect.x < parentRect.x ||
        childRect.y < parentRect.y ||
        childRect.x + childRect.width > parentRect.x + parentRect.width ||
        childRect.y + childRect.height > parentRect.y + parentRect.height) {
        ReportFailure("Child is not fully contained in parent");
    }
}

void UIAssert::DoesOverlap(Control* control1, Control* control2, bool shouldOverlap) {
    if (!control1 || !control2) {
        ReportFailure("One or both controls are null");
    }
    
    auto rect1 = ControlTestHook::GetRenderRect(control1);
    auto rect2 = ControlTestHook::GetRenderRect(control2);
    
    bool overlaps = rect1.Intersects(rect2);
    
    if (overlaps != shouldOverlap) {
        ReportFailure(shouldOverlap ? "Controls should overlap but don't" 
                                    : "Controls should not overlap but do");
    }
}

// ============================================================================
// Rendering Assertions
// ============================================================================
void UIAssert::MatchesBaseline(Window* window, const std::string& baselineName,
                                float threshold, IScreenshotCapture* capture) {
    if (!window) {
        ReportFailure("Window is null");
    }
    
    // This would use BaselineManager to compare
    // Placeholder implementation
    (void)baselineName;
    (void)threshold;
    (void)capture;
}

void UIAssert::MatchesBaseline(Control* control, const std::string& baselineName,
                                float threshold, IScreenshotCapture* capture) {
    if (!control) {
        ReportFailure("Control is null");
    }
    
    // Placeholder implementation
    (void)baselineName;
    (void)threshold;
    (void)capture;
}

} // namespace automation
} // namespace test
} // namespace luaui
