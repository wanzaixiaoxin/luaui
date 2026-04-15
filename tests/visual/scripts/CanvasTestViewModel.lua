-- Canvas Test ViewModel
-- Demonstrates Canvas absolute positioning

Log.info("[CanvasTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "Canvas Layout Test"
ViewModel.TestDescription = "Demonstrates absolute positioning with Canvas.Left and Canvas.Top properties"

-- Control 1 position
ViewModel.Control1X = 50
ViewModel.Control1Y = 30

-- Reset position
function ViewModel:ResetCommand()
    self.Control1X = 50
    self.Control1Y = 30
    Log.info("[CanvasTest] Reset position to (50, 30)")
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
