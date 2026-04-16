-- Grid Test ViewModel - Enhanced
-- Demonstrates Grid with Auto, Star, Pixel sizing, nested grids, and dynamic columns

Log.info("[GridTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "Grid Layout Test"
ViewModel.TestDescription = "Auto/Star/Pixel sizing, ColumnSpan/RowSpan, nested grids, and dynamic column width"

-- Column ratios for dynamic demo
ViewModel.Col1Ratio = 1.0
ViewModel.Col2Ratio = 1.0

-- Computed GridLength strings
ViewModel:DefineComputed("Col1Width",
    {"Col1Ratio"},
    function(self)
        return string.format("%.1f*", self.Col1Ratio)
    end
)

ViewModel:DefineComputed("Col2Width",
    {"Col2Ratio"},
    function(self)
        return string.format("%.1f*", self.Col2Ratio)
    end
)

ViewModel:DefineComputed("Col1RatioText",
    {"Col1Ratio"},
    function(self)
        return string.format("%.1f*", self.Col1Ratio)
    end
)

ViewModel:DefineComputed("Col2RatioText",
    {"Col2Ratio"},
    function(self)
        return string.format("%.1f*", self.Col2Ratio)
    end
)

ViewModel:DefineComputed("ColWidthText",
    {"Col1Ratio", "Col2Ratio"},
    function(self)
        return string.format("Column widths: %.1f* : %.1f*", self.Col1Ratio, self.Col2Ratio)
    end
)

-- Commands
function ViewModel:EqualCommand()
    self.Col1Ratio = 1.0
    self.Col2Ratio = 1.0
    Log.info("[GridTest] Set equal column widths")
end

function ViewModel:ResetCommand()
    self.Col1Ratio = 1.0
    self.Col2Ratio = 1.0
    Log.info("[GridTest] Reset to defaults")
end

-- Register globally
_G.GridTestViewModel = ViewModel

Log.info("[GridTestViewModel] Ready")
