-- Canvas Test ViewModel - Enhanced
-- Demonstrates Canvas with ZIndex, Right/Bottom, and negative coordinates

Log.info("[CanvasTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "Canvas Layout Test"
ViewModel.TestDescription = "Absolute positioning with ZIndex, Right/Bottom, and negative coordinates"

-- Control 1 position
ViewModel.Control1X = 100
ViewModel.Control1Y = 70

-- Reset position
function ViewModel:ResetCommand()
    self.Control1X = 100
    self.Control1Y = 70
    Log.info("[CanvasTest] Reset position to (100, 70)")
end

-- Center position
function ViewModel:CenterCommand()
    self.Control1X = 150
    self.Control1Y = 70
    Log.info("[CanvasTest] Center position at (150, 70)")
end

-- Computed properties
ViewModel:DefineComputed("Control1XText",
    {"Control1X"},
    function(self)
        return string.format("%dpx", self.Control1X)
    end
)

ViewModel:DefineComputed("Control1YText",
    {"Control1Y"},
    function(self)
        return string.format("%dpx", self.Control1Y)
    end
)

-- Register globally
_G.CanvasTestViewModel = ViewModel

Log.info("[CanvasTestViewModel] Ready")
