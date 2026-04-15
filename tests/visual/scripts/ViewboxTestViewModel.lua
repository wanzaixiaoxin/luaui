-- Viewbox Test ViewModel
-- Demonstrates Viewbox scaling

Log.info("[ViewboxTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "Viewbox Layout Test"
ViewModel.TestDescription = "Demonstrates content scaling with different Stretch modes"

-- Properties
ViewModel.Stretch = "Uniform"

-- Commands
function ViewModel:SetNoneCommand()
    self.Stretch = "None"
    Log.info("[ViewboxTest] Set stretch to None")
end

function ViewModel:SetFillCommand()
    self.Stretch = "Fill"
    Log.info("[ViewboxTest] Set stretch to Fill")
end

function ViewModel:SetUniformCommand()
    self.Stretch = "Uniform"
    Log.info("[ViewboxTest] Set stretch to Uniform")
end

function ViewModel:SetUniformToFillCommand()
    self.Stretch = "UniformToFill"
    Log.info("[ViewboxTest] Set stretch to UniformToFill")
end

-- Computed properties
ViewModel:DefineComputed("StretchText",
    {"Stretch"},
    function(self)
        local descriptions = {
            None = "No scaling - content uses original size",
            Fill = "Fill - content stretched to fill container (may distort)",
            Uniform = "Uniform - content scaled proportionally to fit",
            UniformToFill = "UniformToFill - content scaled to fill, may be clipped"
        }
        return string.format("Stretch: %s - %s", self.Stretch, descriptions[self.Stretch] or "")
    end
)

-- Register globally
_G.ViewboxTestViewModel = ViewModel

Log.info("[ViewboxTestViewModel] Ready")
