-- ScrollViewer Test ViewModel - Enhanced
-- Demonstrates ScrollViewer with various scroll bar visibility options

Log.info("[ScrollViewerTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "ScrollViewer Layout Test"
ViewModel.TestDescription = "Scrolling in vertical, horizontal, and both directions with ScrollBar visibility control"

-- ScrollBar visibility state
ViewModel.VScrollVisibility = "Auto"
ViewModel.VisibilityStatus = "Vertical ScrollBar: Auto (show when needed)"

-- Commands for ScrollBar visibility
function ViewModel:SetAutoCommand()
    self.VScrollVisibility = "Auto"
    self.VisibilityStatus = "Vertical ScrollBar: Auto (show when needed)"
    Log.info("[ScrollViewerTestViewModel] Set ScrollBar visibility to Auto")
end

function ViewModel:SetVisibleCommand()
    self.VScrollVisibility = "Visible"
    self.VisibilityStatus = "Vertical ScrollBar: Visible (always show)"
    Log.info("[ScrollViewerTestViewModel] Set ScrollBar visibility to Visible")
end

function ViewModel:SetHiddenCommand()
    self.VScrollVisibility = "Hidden"
    self.VisibilityStatus = "Vertical ScrollBar: Hidden (hide but keep space)"
    Log.info("[ScrollViewerTestViewModel] Set ScrollBar visibility to Hidden")
end

function ViewModel:SetDisabledCommand()
    self.VScrollVisibility = "Disabled"
    self.VisibilityStatus = "Vertical ScrollBar: Disabled (no scroll)"
    Log.info("[ScrollViewerTestViewModel] Set ScrollBar visibility to Disabled")
end

-- Register globally
_G.ScrollViewerTestViewModel = ViewModel

Log.info("[ScrollViewerTestViewModel] Ready")
