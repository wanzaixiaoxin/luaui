-- StackPanel Test ViewModel
-- Demonstrates StackPanel layout with interactive controls

Log.info("[StackPanelTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "StackPanel Layout Test"
ViewModel.TestDescription = "Demonstrates vertical and horizontal stacking with dynamic spacing and item management"

-- Spacing values
ViewModel.VerticalSpacing = 8
ViewModel.HorizontalSpacing = 5

-- Item text
ViewModel.Item1Text = "Item 1 - Red"
ViewModel.Item2Text = "Item 2 - Green"  
ViewModel.Item3Text = "Item 3 - Blue"

-- Dynamic items
ViewModel.DynamicItems = {}
ViewModel.ItemCounter = 0
ViewModel.Orientation = "Vertical" -- "Vertical" or "Horizontal"
ViewModel.MiddleItemVisible = true

-- Toggle visibility
function ViewModel:ToggleVisibilityCommand()
    self.MiddleItemVisible = not self.MiddleItemVisible
    Log.infof("[StackPanelTest] Middle item visibility: %s", tostring(self.MiddleItemVisible))
end

-- Clear all items
function ViewModel:ClearAllCommand()
    self.DynamicItems = {}
    self.ItemCounter = 0
    self.DynamicItems = self.DynamicItems -- Trigger update
    Log.info("[StackPanelTest] Cleared all items")
end

-- Computed text for spacing
function ViewModel:UpdateSpacingText()
    self.VerticalSpacingText = string.format("%dpx", self.VerticalSpacing)
    self.HorizontalSpacingText = string.format("%dpx", self.HorizontalSpacing)
end

-- Add a dynamic item
function ViewModel:AddItemCommand()
    self.ItemCounter = self.ItemCounter + 1
    
    local colors = {
        "#FF5733", "#33FF57", "#3357FF", "#FF33F5", 
        "#F5FF33", "#33FFF5", "#FF3333", "#33FF33"
    }
    local color = colors[(self.ItemCounter % #colors) + 1]
    
    local item = {
        Label = string.format("Dynamic Item %d", self.ItemCounter),
        Color = color,
        Index = self.ItemCounter
    }
    
    table.insert(self.DynamicItems, item)
    self.DynamicItems = self.DynamicItems -- Trigger update
    
    Log.infof("[StackPanelTest] Added item %d", self.ItemCounter)
end

-- Remove last item
function ViewModel:RemoveItemCommand()
    if #self.DynamicItems > 0 then
        table.remove(self.DynamicItems)
        self.DynamicItems = self.DynamicItems -- Trigger update
        Log.info("[StackPanelTest] Removed last item")
    end
end

-- Toggle orientation
function ViewModel:ToggleOrientationCommand()
    if self.Orientation == "Vertical" then
        self.Orientation = "Horizontal"
    else
        self.Orientation = "Vertical"
    end
    Log.infof("[StackPanelTest] Orientation changed to %s", self.Orientation)
end

-- Computed properties
ViewModel:DefineComputed("VerticalSpacingText", 
    {"VerticalSpacing"},
    function(self)
        return string.format("%dpx", self.VerticalSpacing)
    end
)

ViewModel:DefineComputed("HorizontalSpacingText",
    {"HorizontalSpacing"},
    function(self)
        return string.format("%dpx", self.HorizontalSpacing)
    end
)

ViewModel:DefineComputed("OrientationText",
    {"Orientation"},
    function(self)
        return string.format("Current Orientation: %s", self.Orientation)
    end
)

-- Initialize with some items
ViewModel:AddItemCommand()
ViewModel:AddItemCommand()
ViewModel:AddItemCommand()

-- Register globally
_G.StackPanelTestViewModel = ViewModel

Log.info("[StackPanelTestViewModel] Ready")
