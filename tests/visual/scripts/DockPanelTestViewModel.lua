-- DockPanel Test ViewModel - Enhanced
-- Demonstrates DockPanel with multiple same direction, dock order, and LastChildFill

Log.info("[DockPanelTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "DockPanel Layout Test"
ViewModel.TestDescription = "Docking to edges with multiple same direction, dock order, and LastChildFill toggle"

-- Properties
ViewModel.LastChildFill = true

-- Computed properties
ViewModel:DefineComputed("LastChildFillText",
    {"LastChildFill"},
    function(self)
        if self.LastChildFill then
            return "LastChildFill: true - Center fills remaining space"
        else
            return "LastChildFill: false - Center uses its own size"
        end
    end
)

-- Register globally
_G.DockPanelTestViewModel = ViewModel

Log.info("[DockPanelTestViewModel] Ready")
