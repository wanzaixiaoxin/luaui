-- TestRunner ViewModel - Production MVVM Demo
-- Demonstrates: Nested properties, Collection binding, SelectedItem, ComboBox

Log.info("[TestRunnerViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- =============================================================================
-- Properties
-- =============================================================================

-- Test data
ViewModel.Tests = {}
ViewModel.FilteredTests = {}
ViewModel.FilterText = ""
ViewModel.ShowLayoutTests = true
ViewModel.ShowControlTests = true
ViewModel.ShowMvvmTests = true
ViewModel.SelectedTest = nil
ViewModel.SelectedIndex = 0  -- SelectedItem binding for ListBox

-- Status
ViewModel.TestCountText = "Loading tests..."
ViewModel.StatusText = "Ready"

-- Nested property demo (User.Profile.Name style)
ViewModel.User = {
    Name = "Test User",
    Profile = {
        Name = "Developer",
        Level = 5,
        Settings = {
            Theme = "Dark"
        }
    }
}

-- ComboBox data
ViewModel.Categories = {"All", "Layout", "Controls", "MVVM"}
ViewModel.SelectedCategory = 0  -- ComboBox SelectedIndex

-- =============================================================================
-- Commands
-- =============================================================================

function ViewModel:RunSelectedTestCommand()
    if self.SelectedTest then
        self:RunTest(self.SelectedTest)
    elseif #self.FilteredTests > 0 and self.SelectedIndex >= 0 then
        local index = self.SelectedIndex + 1  -- Lua arrays are 1-based
        if index <= #self.FilteredTests then
            self:RunTest(self.FilteredTests[index])
        end
    else
        Log.warn("[TestRunner] No test selected")
    end
end

function ViewModel:ClearFilterCommand()
    Log.info("[TestRunner] Clearing filters")
    self.FilterText = ""
    self.ShowLayoutTests = true
    self.ShowControlTests = true
    self.ShowMvvmTests = true
    self.SelectedCategory = 0
    self:ApplyFilter()
end

function ViewModel:RefreshCommand()
    Log.info("[TestRunner] Refreshing test list")
    self.StatusText = "Refreshing..."
    self:InitializeTests()
    self.StatusText = "Refreshed at " .. os.date("%H:%M:%S")
end

-- Add a test item dynamically (demonstrates collection update)
function ViewModel:AddTestCommand()
    Log.info("[TestRunner] Adding new test dynamically")
    local newTest = {
        Name = "Dynamic Test " .. os.time(),
        Description = "Auto-generated test",
        Category = "Controls"
    }
    table.insert(self.Tests, newTest)
    self:ApplyFilter()
    self.StatusText = "Added new test, total: " .. #self.Tests
end

-- Remove selected test
function ViewModel:RemoveTestCommand()
    if self.SelectedIndex >= 0 then
        local index = self.SelectedIndex + 1
        if index <= #self.FilteredTests then
            local test = self.FilteredTests[index]
            Log.info("[TestRunner] Removing test: " .. test.Name)
            
            -- Remove from Tests array
            for i, t in ipairs(self.Tests) do
                if t == test then
                    table.remove(self.Tests, i)
                    break
                end
            end
            
            self.SelectedIndex = -1
            self:ApplyFilter()
        end
    end
end

-- =============================================================================
-- Methods
-- =============================================================================

function ViewModel:InitializeTests()
    self.Tests = {
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
    
    self:ApplyFilter()
    Log.infof("[TestRunnerViewModel] Loaded %d tests", #self.Tests)
end

function ViewModel:ApplyFilter()
    local filtered = {}
    local filterText = string.lower(self.FilterText or "")
    local categoryFilter = "All"
    
    -- Get category from ComboBox selection
    if self.SelectedCategory >= 0 and self.SelectedCategory < #self.Categories then
        categoryFilter = self.Categories[self.SelectedCategory + 1]
    end
    
    for _, test in ipairs(self.Tests) do
        local categoryMatch = false
        if categoryFilter == "All" then
            categoryMatch = true
        elseif test.Category == "Layout" and (self.ShowLayoutTests or categoryFilter == "Layout") then
            categoryMatch = true
        elseif test.Category == "Controls" and (self.ShowControlTests or categoryFilter == "Controls") then
            categoryMatch = true
        elseif test.Category == "MVVM" and (self.ShowMvvmTests or categoryFilter == "MVVM") then
            categoryMatch = true
        end
        
        local textMatch = filterText == "" or 
            string.find(string.lower(test.Name), filterText) or
            string.find(string.lower(test.Description), filterText)
        
        if categoryMatch and textMatch then
            table.insert(filtered, test)
        end
    end
    
    self.FilteredTests = filtered
    self.TestCountText = string.format("%d tests available", #filtered)
    self:UpdateStatus()
end

function ViewModel:RunTest(test)
    self.SelectedTest = test
    self.StatusText = string.format("Running: %s...", test.Name)
    Log.infof("[TestRunner] Running test: %s", test.Name)
    
    if _G.OnRunTest then
        _G.OnRunTest(test.LayoutFile or "", test.ScriptFile or "")
    end
end

function ViewModel:UpdateStatus()
    -- Demonstrate nested property access
    local profileName = self.User.Profile.Name
    local theme = self.User.Profile.Settings.Theme
    self.StatusText = string.format("User: %s (%s) | Theme: %s | Tests: %d/%d", 
        self.User.Name, profileName, theme, #self.FilteredTests, #self.Tests)
end

-- =============================================================================
-- Property Change Handlers
-- =============================================================================

function ViewModel:OnFilterTextChanged()
    self:ApplyFilter()
end

function ViewModel:OnShowLayoutTestsChanged()
    self:ApplyFilter()
end

function ViewModel:OnShowControlTestsChanged()
    self:ApplyFilter()
end

function ViewModel:OnShowMvvmTestsChanged()
    self:ApplyFilter()
end

function ViewModel:OnSelectedCategoryChanged()
    Log.infof("[TestRunner] Category changed to: %s", self.Categories[self.SelectedCategory + 1] or "All")
    self:ApplyFilter()
end

function ViewModel:OnSelectedIndexChanged()
    Log.infof("[TestRunner] SelectedIndex changed to: %d", self.SelectedIndex)
    if self.SelectedIndex >= 0 and self.SelectedIndex < #self.FilteredTests then
        local test = self.FilteredTests[self.SelectedIndex + 1]
        self.StatusText = "Selected: " .. test.Name
    end
end

-- =============================================================================
-- Initialization
-- =============================================================================

ViewModel:InitializeTests()
ViewModel:UpdateStatus()

-- Register globally
_G.TestRunnerViewModel = ViewModel

Log.info("[TestRunnerViewModel] Ready")
