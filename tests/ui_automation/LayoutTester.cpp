// LayoutTester.cpp - Layout validation and performance testing

#include "UIAutomationFramework.h"
#include "TestHooks.h"
#include "Control.h"
#include "Panel.h"
#include "Window.h"
#include <chrono>

namespace luaui {
namespace test {
namespace automation {

// ============================================================================
// Layout Validation
// ============================================================================
std::vector<LayoutTester::LayoutViolation> LayoutTester::ValidateLayout(Control* root) {
    std::vector<LayoutViolation> violations;
    
    if (!root) return violations;
    
    // Check for negative sizes
    auto rect = ControlTestHook::GetRenderRect(root);
    if (rect.width < 0 || rect.height < 0) {
        LayoutViolation v;
        v.message = "Control has negative size";
        v.control = root;
        violations.push_back(v);
    }
    
    // Check for NaN positions
    if (rect.x != rect.x || rect.y != rect.y) {  // NaN check
        LayoutViolation v;
        v.message = "Control has NaN position";
        v.control = root;
        violations.push_back(v);
    }
    
    // Check for zero size on visible controls
    if (root->GetIsVisible() && rect.width == 0 && rect.height == 0) {
        LayoutViolation v;
        v.message = "Visible control has zero size";
        v.control = root;
        violations.push_back(v);
    }
    
    // Recursively check children
    if (auto* panel = dynamic_cast<controls::Panel*>(root)) {
        auto children = ControlTestHook::GetChildren(panel);
        for (const auto& child : children) {
            auto childViolations = ValidateLayout(dynamic_cast<Control*>(child.get()));
            violations.insert(violations.end(), childViolations.begin(), childViolations.end());
        }
    }
    
    return violations;
}

bool LayoutTester::HasClippingIssues(Control* control) {
    if (!control) return false;
    
    auto rect = ControlTestHook::GetRenderRect(control);
    
    // Check parent bounds
    auto parent = std::dynamic_pointer_cast<Control>(control->GetParent());
    if (parent) {
        auto parentRect = ControlTestHook::GetRenderRect(parent.get());
        
        // Control extends outside parent
        if (rect.x < parentRect.x ||
            rect.y < parentRect.y ||
            rect.x + rect.width > parentRect.x + parentRect.width ||
            rect.y + rect.height > parentRect.y + parentRect.height) {
            return true;
        }
    }
    
    return false;
}

bool LayoutTester::HasOverflow(controls::Panel* panel) {
    if (!panel) return false;
    
    auto panelRect = ControlTestHook::GetRenderRect(panel);
    auto children = ControlTestHook::GetChildren(panel);
    
    for (const auto& child : children) {
        auto childRect = ControlTestHook::GetRenderRect(dynamic_cast<Control*>(child.get()));
        
        // Check if child extends outside panel
        if (childRect.x + childRect.width > panelRect.x + panelRect.width ||
            childRect.y + childRect.height > panelRect.y + panelRect.height) {
            return true;
        }
    }
    
    return false;
}

bool LayoutTester::IsCropped(Control* control) {
    if (!control) return false;
    
    // Check if control is partially or fully outside parent's bounds
    return HasClippingIssues(control);
}

// ============================================================================
// Performance Measurement
// ============================================================================
LayoutTester::LayoutPerformance LayoutTester::MeasurePerformance(Window* window) {
    LayoutPerformance perf = {0, 0, 0};
    
    if (!window) return perf;
    
    auto* tw = dynamic_cast<TestWindow*>(window);
    if (!tw) return perf;
    
    // Count controls
    std::function<int(Control*)> countControls = [&](Control* root) -> int {
        if (!root) return 0;
        
        int count = 1;
        if (auto* panel = dynamic_cast<controls::Panel*>(root)) {
            auto children = ControlTestHook::GetChildren(panel);
            for (const auto& child : children) {
                count += countControls(dynamic_cast<Control*>(child.get()));
            }
        }
        return count;
    };
    
    auto* root = tw->GetRootControl();
    perf.controlCount = countControls(root);
    
    // Measure layout time
    auto start = std::chrono::high_resolution_clock::now();
    
    // Force multiple layout passes to get measurable time
    for (int i = 0; i < 100; i++) {
        tw->InvalidateLayout();
        tw->UpdateLayout();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double totalMs = duration.count() / 1000.0;
    double avgMs = totalMs / 100.0;
    
    // Estimate measure/arrange split (typically 40/60)
    perf.measureTimeMs = avgMs * 0.4;
    perf.arrangeTimeMs = avgMs * 0.6;
    
    return perf;
}

} // namespace automation
} // namespace test
} // namespace luaui
