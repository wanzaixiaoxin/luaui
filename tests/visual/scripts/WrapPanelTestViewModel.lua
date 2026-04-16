-- WrapPanel Test ViewModel - Enhanced
-- Demonstrates WrapPanel with Spacing, ItemWidth/ItemHeight, and different sizes

Log.info("[WrapPanelTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "WrapPanel Layout Test"
ViewModel.TestDescription = "Auto-wrap layout with Spacing, ItemWidth/ItemHeight, and varying item sizes"

-- Properties
ViewModel.Spacing = 8

-- Commands
function ViewModel:NoSpacingCommand()
    self.Spacing = 0
    Log.info("[WrapPanelTest] Set spacing to 0")
end

function ViewModel:DefaultSpacingCommand()
    self.Spacing = 8
    Log.info("[WrapPanelTest] Set spacing to 8")
end

function ViewModel:LargeSpacingCommand()
    self.Spacing = 20
    Log.info("[WrapPanelTest] Set spacing to 20")
end

-- Computed properties
ViewModel:DefineComputed("SpacingText",
    {"Spacing"},
    function(self)
        return string.format("Spacing: %dpx", self.Spacing)
    end
)

-- Register globally
_G.WrapPanelTestViewModel = ViewModel

Log.info("[WrapPanelTestViewModel] Ready")
