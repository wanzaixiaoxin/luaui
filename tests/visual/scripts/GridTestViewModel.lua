-- Grid Test ViewModel
-- Demonstrates Grid layout with interactive sizing

Log.info("[GridTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "Grid Layout Test"
ViewModel.TestDescription = "Demonstrates star sizing, column spanning, and dynamic column width adjustments"

-- Cell text
ViewModel.Cell00Text = "Star sizing distributes available space proportionally"
ViewModel.Cell01Text = "Each cell gets equal space with 1*"
ViewModel.Cell10Text = "Grid arranges children in rows and columns"
ViewModel.Cell11Text = "Supports spanning across multiple cells"

-- Column widths (star values)
ViewModel.Col1Width = 1.0
ViewModel.Col2Width = 1.0

-- Commands
function ViewModel:EqualColumnsCommand()
    self.Col1Width = 1.0
    self.Col2Width = 1.0
    Log.info("[GridTest] Set equal column widths")
end

function ViewModel:ResetCommand()
    self.Col1Width = 1.0
    self.Col2Width = 1.0
    self.Cell00Text = "Star sizing distributes available space proportionally"
    self.Cell01Text = "Each cell gets equal space with 1*"
    self.Cell10Text = "Grid arranges children in rows and columns"
    self.Cell11Text = "Supports spanning across multiple cells"
    Log.info("[GridTest] Reset to defaults")
end

-- Computed properties
ViewModel:DefineComputed("Col1WidthText",
    {"Col1Width"},
    function(self)
        return string.format("%.1f*", self.Col1Width)
    end
)

ViewModel:DefineComputed("Col2WidthText",
    {"Col2Width"},
    function(self)
        return string.format("%.1f*", self.Col2Width)
    end
)

-- Example of cell text that updates based on width
ViewModel:DefineComputed("Cell00DynamicText",
    {"Col1Width", "Col2Width"},
    function(self)
        local ratio = self.Col1Width / (self.Col1Width + self.Col2Width)
        return string.format("Column 1 ratio: %.0f%%", ratio * 100)
    end
)

-- Register globally
_G.GridTestViewModel = ViewModel

Log.info("[GridTestViewModel] Ready")
