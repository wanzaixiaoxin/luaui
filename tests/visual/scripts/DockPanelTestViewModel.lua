-- DockPanel Test ViewModel
-- Demonstrates DockPanel docking layout

Log.info("[DockPanelTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "DockPanel Layout Test"
ViewModel.TestDescription = "Demonstrates docking to edges (Top, Bottom, Left, Right) with LastChildFill property"

-- Properties
ViewModel.LastChildFill = true

-- Computed properties
ViewModel:DefineComputed("LastChildFillText",
    {"LastChildFill"},
    function(self)
        return self.LastChildFill and "Last child fills remaining space" or "Last child uses its own size"
    end
)

-- Register globally
_G.DockPanelTestViewModel = ViewModel

Log.info("[DockPanelTestViewModel] Ready")
