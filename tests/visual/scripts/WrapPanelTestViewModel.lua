-- WrapPanel Test ViewModel
-- Demonstrates WrapPanel auto-wrap layout

Log.info("[WrapPanelTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "WrapPanel Layout Test"
ViewModel.TestDescription = "Demonstrates automatic wrapping when items exceed available space"

-- Properties
ViewModel.Orientation = "Horizontal"

-- Commands
function ViewModel:SetHorizontalCommand()
    self.Orientation = "Horizontal"
    Log.info("[WrapPanelTest] Set orientation to Horizontal")
end

function ViewModel:SetVerticalCommand()
    self.Orientation = "Vertical"
    Log.info("[WrapPanelTest] Set orientation to Vertical")
end

-- Computed properties
ViewModel:DefineComputed("OrientationText",
    {"Orientation"},
    function(self)
        return string.format("Current Orientation: %s (items wrap when space runs out)", self.Orientation)
    end
)

-- Register globally
_G.WrapPanelTestViewModel = ViewModel

Log.info("[WrapPanelTestViewModel] Ready")
