-- TestRunner ViewModel
-- Visual test suite runner with real layout demonstrations

Log.info("[TestRunnerViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Current test info
ViewModel.CurrentTestName = "StackPanel"
ViewModel.TestTitle = "StackPanel Layout"
ViewModel.TestDescription = "Vertical and horizontal stacking with spacing"

-- Visibility bindings (using "Visible" and "Collapsed" strings)
ViewModel.StackPanelVisible = "Visible"
ViewModel.GridVisible = "Collapsed"
ViewModel.CanvasVisible = "Collapsed"
ViewModel.DockPanelVisible = "Collapsed"
ViewModel.WrapPanelVisible = "Collapsed"
ViewModel.ScrollViewerVisible = "Collapsed"
ViewModel.ViewboxVisible = "Collapsed"

-- Test Commands
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
    Log.info("[TestRunner] Showing StackPanel demo")
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
    self.TestDescription = "Row/column layout with star sizing and spanning"
    self:EndBatch()
    Log.info("[TestRunner] Showing Grid demo")
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
    self.TestDescription = "Absolute positioning with Canvas.Left/Top"
    self:EndBatch()
    Log.info("[TestRunner] Showing Canvas demo")
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
    self.TestDescription = "Dock to edges with LastChildFill"
    self:EndBatch()
    Log.info("[TestRunner] Showing DockPanel demo")
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
    self.TestDescription = "Auto wrap when space runs out"
    self:EndBatch()
    Log.info("[TestRunner] Showing WrapPanel demo")
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
    self.TestDescription = "Scrollable content container"
    self:EndBatch()
    Log.info("[TestRunner] Showing ScrollViewer demo")
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
    Log.info("[TestRunner] Showing Viewbox demo")
end

-- Register globally
_G.TestRunnerViewModel = ViewModel

Log.info("[TestRunnerViewModel] Ready - showing StackPanel demo by default")
