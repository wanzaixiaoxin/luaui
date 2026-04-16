-- TestRunner ViewModel - Unified
-- Contains all properties and commands for all layout tests

Log.info("[TestRunnerViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- ============================================
-- Current Test Info
-- ============================================
ViewModel.CurrentTestName = "StackPanel"
ViewModel.TestTitle = "StackPanel Layout"
ViewModel.TestDescription = "Vertical and horizontal stacking with spacing"

-- Visibility bindings
ViewModel.StackPanelVisible = "Visible"
ViewModel.GridVisible = "Collapsed"
ViewModel.CanvasVisible = "Collapsed"
ViewModel.DockPanelVisible = "Collapsed"
ViewModel.WrapPanelVisible = "Collapsed"
ViewModel.ScrollViewerVisible = "Collapsed"
ViewModel.ViewboxVisible = "Collapsed"

-- ============================================
-- StackPanel Properties
-- ============================================
ViewModel.StackPanelSpacing = 8
ViewModel.StackPanelOrientation = "Vertical"

function ViewModel:SetSpacing0()
    self.StackPanelSpacing = 0
end

function ViewModel:SetSpacing8()
    self.StackPanelSpacing = 8
end

function ViewModel:SetSpacing16()
    self.StackPanelSpacing = 16
end

function ViewModel:SetVerticalOrientation()
    self.StackPanelOrientation = "Vertical"
end

function ViewModel:SetHorizontalOrientation()
    self.StackPanelOrientation = "Horizontal"
end

ViewModel:DefineComputed("StackPanelSpacingText",
    {"StackPanelSpacing"},
    function(self)
        return string.format("Spacing: %dpx", self.StackPanelSpacing)
    end
)

-- ============================================
-- Grid Properties
-- ============================================
ViewModel.GridCol1Ratio = 1.0
ViewModel.GridCol2Ratio = 1.0

ViewModel:DefineComputed("GridCol1Width",
    {"GridCol1Ratio"},
    function(self)
        return string.format("%.1f*", self.GridCol1Ratio)
    end
)

ViewModel:DefineComputed("GridCol2Width",
    {"GridCol2Ratio"},
    function(self)
        return string.format("%.1f*", self.GridCol2Ratio)
    end
)

ViewModel:DefineComputed("GridCol1RatioText",
    {"GridCol1Ratio"},
    function(self)
        return string.format("%.1f*", self.GridCol1Ratio)
    end
)

ViewModel:DefineComputed("GridCol2RatioText",
    {"GridCol2Ratio"},
    function(self)
        return string.format("%.1f*", self.GridCol2Ratio)
    end
)

ViewModel:DefineComputed("GridColWidthText",
    {"GridCol1Ratio", "GridCol2Ratio"},
    function(self)
        return string.format("Column widths: %.1f* : %.1f*", self.GridCol1Ratio, self.GridCol2Ratio)
    end
)

function ViewModel:GridEqualCommand()
    self.GridCol1Ratio = 1.0
    self.GridCol2Ratio = 1.0
end

function ViewModel:GridResetCommand()
    self.GridCol1Ratio = 1.0
    self.GridCol2Ratio = 1.0
end

-- ============================================
-- Canvas Properties
-- ============================================
ViewModel.CanvasX = 100
ViewModel.CanvasY = 70

ViewModel:DefineComputed("CanvasXText",
    {"CanvasX"},
    function(self)
        return string.format("%dpx", self.CanvasX)
    end
)

ViewModel:DefineComputed("CanvasYText",
    {"CanvasY"},
    function(self)
        return string.format("%dpx", self.CanvasY)
    end
)

function ViewModel:CanvasResetCommand()
    self.CanvasX = 100
    self.CanvasY = 70
end

function ViewModel:CanvasCenterCommand()
    self.CanvasX = 150
    self.CanvasY = 70
end

-- ============================================
-- DockPanel Properties
-- ============================================
ViewModel.DockLastChildFill = true

ViewModel:DefineComputed("DockLastChildFillText",
    {"DockLastChildFill"},
    function(self)
        if self.DockLastChildFill then
            return "LastChildFill: true - Center fills remaining space"
        else
            return "LastChildFill: false - Center uses its own size"
        end
    end
)

-- ============================================
-- WrapPanel Properties
-- ============================================
ViewModel.WrapSpacing = 8

function ViewModel:WrapNoSpacingCommand()
    self.WrapSpacing = 0
end

function ViewModel:WrapDefaultSpacingCommand()
    self.WrapSpacing = 8
end

function ViewModel:WrapLargeSpacingCommand()
    self.WrapSpacing = 20
end

ViewModel:DefineComputed("WrapSpacingText",
    {"WrapSpacing"},
    function(self)
        return string.format("Spacing: %dpx", self.WrapSpacing)
    end
)

-- ============================================
-- ScrollViewer Properties
-- ============================================
ViewModel.ScrollVVisibility = "Auto"
ViewModel.ScrollVisibilityStatus = "Vertical ScrollBar: Auto (show when needed)"

function ViewModel:ScrollSetAutoCommand()
    self.ScrollVVisibility = "Auto"
    self.ScrollVisibilityStatus = "Vertical ScrollBar: Auto (show when needed)"
end

function ViewModel:ScrollSetVisibleCommand()
    self.ScrollVVisibility = "Visible"
    self.ScrollVisibilityStatus = "Vertical ScrollBar: Visible (always show)"
end

function ViewModel:ScrollSetHiddenCommand()
    self.ScrollVVisibility = "Hidden"
    self.ScrollVisibilityStatus = "Vertical ScrollBar: Hidden (hide but keep space)"
end

function ViewModel:ScrollSetDisabledCommand()
    self.ScrollVVisibility = "Disabled"
    self.ScrollVisibilityStatus = "Vertical ScrollBar: Disabled (no scroll)"
end

-- ============================================
-- Viewbox Properties
-- ============================================
ViewModel.ViewboxStretch = "Uniform"

function ViewModel:SetStretchNone()
    self.ViewboxStretch = "None"
end

function ViewModel:SetStretchFill()
    self.ViewboxStretch = "Fill"
end

function ViewModel:SetStretchUniform()
    self.ViewboxStretch = "Uniform"
end

function ViewModel:SetStretchUniformToFill()
    self.ViewboxStretch = "UniformToFill"
end

-- ============================================
-- Test Switch Commands
-- ============================================
function ViewModel:ShowStackPanel()
    self:BeginBatch()
    self.StackPanelVisible = "Visible"
    self.GridVisible = "Collapsed"
    self.CanvasVisible = "Collapsed"
    self.DockPanelVisible = "Collapsed"
    self.WrapPanelVisible = "Collapsed"
    self.ScrollViewerVisible = "Collapsed"
    self.ViewboxVisible = "Collapsed"
    self.CurrentTestName = "StackPanel"
    self.TestTitle = "StackPanel Layout"
    self.TestDescription = "Vertical and horizontal stacking with spacing"
    self:EndBatch()
end

function ViewModel:ShowGrid()
    self:BeginBatch()
    self.StackPanelVisible = "Collapsed"
    self.GridVisible = "Visible"
    self.CanvasVisible = "Collapsed"
    self.DockPanelVisible = "Collapsed"
    self.WrapPanelVisible = "Collapsed"
    self.ScrollViewerVisible = "Collapsed"
    self.ViewboxVisible = "Collapsed"
    self.CurrentTestName = "Grid"
    self.TestTitle = "Grid Layout"
    self.TestDescription = "Auto/Star/Pixel sizing, ColumnSpan/RowSpan, nested grids"
    self:EndBatch()
end

function ViewModel:ShowCanvas()
    self:BeginBatch()
    self.StackPanelVisible = "Collapsed"
    self.GridVisible = "Collapsed"
    self.CanvasVisible = "Visible"
    self.DockPanelVisible = "Collapsed"
    self.WrapPanelVisible = "Collapsed"
    self.ScrollViewerVisible = "Collapsed"
    self.ViewboxVisible = "Collapsed"
    self.CurrentTestName = "Canvas"
    self.TestTitle = "Canvas Layout"
    self.TestDescription = "ZIndex, Right/Bottom positioning, negative coordinates"
    self:EndBatch()
end

function ViewModel:ShowDockPanel()
    self:BeginBatch()
    self.StackPanelVisible = "Collapsed"
    self.GridVisible = "Collapsed"
    self.CanvasVisible = "Collapsed"
    self.DockPanelVisible = "Visible"
    self.WrapPanelVisible = "Collapsed"
    self.ScrollViewerVisible = "Collapsed"
    self.ViewboxVisible = "Collapsed"
    self.CurrentTestName = "DockPanel"
    self.TestTitle = "DockPanel Layout"
    self.TestDescription = "Multiple same direction, dock order, LastChildFill"
    self:EndBatch()
end

function ViewModel:ShowWrapPanel()
    self:BeginBatch()
    self.StackPanelVisible = "Collapsed"
    self.GridVisible = "Collapsed"
    self.CanvasVisible = "Collapsed"
    self.DockPanelVisible = "Collapsed"
    self.WrapPanelVisible = "Visible"
    self.ScrollViewerVisible = "Collapsed"
    self.ViewboxVisible = "Collapsed"
    self.CurrentTestName = "WrapPanel"
    self.TestTitle = "WrapPanel Layout"
    self.TestDescription = "Spacing, ItemWidth/ItemHeight, varying item sizes"
    self:EndBatch()
end

function ViewModel:ShowScrollViewer()
    self:BeginBatch()
    self.StackPanelVisible = "Collapsed"
    self.GridVisible = "Collapsed"
    self.CanvasVisible = "Collapsed"
    self.DockPanelVisible = "Collapsed"
    self.WrapPanelVisible = "Collapsed"
    self.ScrollViewerVisible = "Visible"
    self.ViewboxVisible = "Collapsed"
    self.CurrentTestName = "ScrollViewer"
    self.TestTitle = "ScrollViewer Layout"
    self.TestDescription = "Vertical/horizontal scroll, ScrollBarVisibility control"
    self:EndBatch()
end

function ViewModel:ShowViewbox()
    self:BeginBatch()
    self.StackPanelVisible = "Collapsed"
    self.GridVisible = "Collapsed"
    self.CanvasVisible = "Collapsed"
    self.DockPanelVisible = "Collapsed"
    self.WrapPanelVisible = "Collapsed"
    self.ScrollViewerVisible = "Collapsed"
    self.ViewboxVisible = "Visible"
    self.CurrentTestName = "Viewbox"
    self.TestTitle = "Viewbox Layout"
    self.TestDescription = "Content scaling with Stretch modes"
    self:EndBatch()
end

-- Register globally
_G.TestRunnerViewModel = ViewModel

Log.info("[TestRunnerViewModel] Ready")
