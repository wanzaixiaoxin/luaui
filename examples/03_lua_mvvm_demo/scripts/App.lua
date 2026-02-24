-- App.lua - MVVM ViewModel
-- This file is loaded by C++ host

-- ========================================
-- Framework Classes
-- ========================================

ViewModel = {}
ViewModel.__index = ViewModel

function ViewModel.new()
    local self = setmetatable({}, ViewModel)
    self._listeners = {}
    return self
end

function ViewModel:PropertyChanged(propertyName)
    if self._listeners[propertyName] then
        for _, callback in ipairs(self._listeners[propertyName]) do
            callback(self[propertyName])
        end
    end
end

-- ========================================
-- Main ViewModel
-- ========================================

MainViewModel = {}
MainViewModel.__index = MainViewModel
setmetatable(MainViewModel, ViewModel)

function MainViewModel.new()
    local self = setmetatable(ViewModel.new(), MainViewModel)
    
    -- Properties (bound to XML)
    self.Title = "LuaUI MVVM Demo"
    self.Description = "XML View + Lua ViewModel + C++ Host"
    self.Counter = 0
    self.IsFeatureEnabled = false
    self.FeatureStatusText = "Feature is currently disabled"
    self.ItemCount = 2
    self.Items = {
        { Name = "Item A", Value = 100 },
        { Name = "Item B", Value = 200 }
    }
    self.Status = "Ready"
    
    return self
end

function MainViewModel:UpdateStatus()
    self.Status = string.format("Counter: %d | Items: %d | Feature: %s",
        self.Counter,
        self.ItemCount,
        self.IsFeatureEnabled and "ON" or "OFF"
    )
end

-- Commands (bound to Button Click in XML)
function MainViewModel:IncrementCommand()
    self.Counter = self.Counter + 1
    self:UpdateStatus()
    print("Counter: " .. self.Counter)
    -- Note: In full implementation, this would trigger UI refresh
end

function MainViewModel:DecrementCommand()
    self.Counter = self.Counter - 1
    self:UpdateStatus()
end

function MainViewModel:ToggleCommand()
    self.IsFeatureEnabled = not self.IsFeatureEnabled
    if self.IsFeatureEnabled then
        self.FeatureStatusText = "Feature is currently enabled"
    else
        self.FeatureStatusText = "Feature is currently disabled"
    end
    self:UpdateStatus()
    print("Feature: " .. tostring(self.IsFeatureEnabled))
end

function MainViewModel:AddItemCommand()
    local count = #self.Items + 1
    table.insert(self.Items, { Name = "Item " .. count, Value = math.random(10, 99) })
    self.ItemCount = #self.Items
    self:UpdateStatus()
end

function MainViewModel:ClearItemsCommand()
    self.Items = {}
    self.ItemCount = 0
    self:UpdateStatus()
end

function MainViewModel:ResetCommand()
    self.Counter = 0
    self.IsFeatureEnabled = false
    self.FeatureStatusText = "Feature is currently disabled"
    self.Items = {
        { Name = "Item A", Value = 100 },
        { Name = "Item B", Value = 200 }
    }
    self.ItemCount = 2
    self:UpdateStatus()
    print("Reset")
end

-- ========================================
-- Create Global Instance
-- ========================================

_G.ViewModelInstance = MainViewModel.new()
print("ViewModel loaded")
