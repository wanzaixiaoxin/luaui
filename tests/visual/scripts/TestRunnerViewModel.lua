-- TestRunner ViewModel - 增量集合更新演示

Log.info("[TestRunnerViewModel] Initializing with ObservableCollection...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- =============================================================================
-- Properties - 使用 ObservableCollection 支持增量更新
-- =============================================================================

-- 使用 ObservableCollection 替代普通 table
ViewModel.Tests = ObservableCollection.new()
ViewModel.FilteredTests = ObservableCollection.new()

ViewModel.FilterText = ""
ViewModel.ShowLayoutTests = true
ViewModel.ShowControlTests = true
ViewModel.ShowMvvmTests = true

-- Status
ViewModel.TestCountText = "Loading tests..."
ViewModel.StatusText = "Ready"

-- =============================================================================
-- Commands
-- =============================================================================

-- 添加测试 - 演示增量更新
function ViewModel:AddTestCommand()
    Log.info("[TestRunner] Adding new test dynamically (incremental update)")
    
    local newTest = {
        Name = "Dynamic Test " .. os.time(),
        Description = "Auto-generated test",
        Category = "Controls"
    }
    
    -- 增量添加 - 只触发单个 Add 通知
    ViewModel.Tests:Add(newTest)
    
    self:UpdateStatus()
end

-- 批量添加 - 演示批量更新
function ViewModel:AddMultipleTestsCommand()
    Log.info("[TestRunner] Adding multiple tests (batch update)")
    
    for i = 1, 5 do
        local newTest = {
            Name = "Batch Test " .. i,
            Description = "Batch generated",
            Category = "Layout"
        }
        ViewModel.Tests:Add(newTest)
    end
    
    self:UpdateStatus()
end

-- 移除最后一个测试
function ViewModel:RemoveLastTestCommand()
    local count = ViewModel.Tests:Count()
    if count > 0 then
        Log.infof("[TestRunner] Removing test at index %d", count)
        ViewModel.Tests:RemoveAt(count)
        self:UpdateStatus()
    end
end

-- 清空所有测试
function ViewModel:ClearTestsCommand()
    Log.info("[TestRunner] Clearing all tests")
    ViewModel.Tests:Clear()
    self:UpdateStatus()
end

-- 刷新列表
function ViewModel:RefreshCommand()
    Log.info("[TestRunner] Refreshing...")
    self.StatusText = "Refreshing..."
    
    -- 模拟异步加载
    self:InitializeTests()
    
    self.StatusText = "Refreshed at " .. os.date("%H:%M:%S")
end

-- =============================================================================
-- Methods
-- =============================================================================

function ViewModel:InitializeTests()
    -- 清空现有数据
    ViewModel.Tests:Clear()
    
    -- 批量添加初始测试数据
    local initialTests = {
        { Name = "StackPanel Layout", Description = "Vertical and horizontal stacking", Category = "Layout" },
        { Name = "Grid Layout", Description = "2x2 grid with star sizing", Category = "Layout" },
        { Name = "Canvas Positioning", Description = "Absolute positioning", Category = "Layout" },
        { Name = "Button Controls", Description = "Various button styles", Category = "Controls" },
        { Name = "CheckBox & RadioButton", Description = "Selection controls", Category = "Controls" },
        { Name = "Slider & ProgressBar", Description = "Value controls", Category = "Controls" },
        { Name = "TextBox Input", Description = "Text input with validation", Category = "Controls" },
        { Name = "MVVM Binding", Description = "One-way, two-way binding", Category = "MVVM" },
        { Name = "Converter Demo", Description = "Boolean to visibility", Category = "MVVM" },
        { Name = "TreeView Control", Description = "Hierarchical data", Category = "Controls" },
        { Name = "DataGrid Control", Description = "Tabular data", Category = "Controls" }
    }
    
    for _, test in ipairs(initialTests) do
        ViewModel.Tests:Add(test)
    end
    
    self:UpdateStatus()
    Log.infof("[TestRunnerViewModel] Loaded %d tests using ObservableCollection", ViewModel.Tests:Count())
end

function ViewModel:UpdateStatus()
    self.TestCountText = string.format("%d tests available", ViewModel.Tests:Count())
    self.StatusText = string.format("Total: %d tests | ObservableCollection with incremental updates", 
        ViewModel.Tests:Count())
end

-- =============================================================================
-- Property Change Handlers
-- =============================================================================

function ViewModel:OnShowLayoutTestsChanged()
    Log.info("[TestRunner] Filter changed, would apply filter here")
end

-- =============================================================================
-- Initialization
-- =============================================================================

ViewModel:InitializeTests()

-- Register globally
_G.TestRunnerViewModel = ViewModel

Log.info("[TestRunnerViewModel] Ready with ObservableCollection support")
