-- ViewModel.lua
-- MVVM ViewModel with data and commands

-- Framework (embedded for simplicity)
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

function ViewModel:Subscribe(propertyName, callback)
    if not self._listeners[propertyName] then
        self._listeners[propertyName] = {}
    end
    table.insert(self._listeners[propertyName], callback)
end

Command = {}
Command.__index = Command

function Command.new(execute, canExecute)
    local self = setmetatable({}, Command)
    self._execute = execute or function() end
    self._canExecute = canExecute or function() return true end
    return self
end

function Command:Execute(parameter)
    if self:CanExecute(parameter) then
        self._execute(parameter)
    end
end

function Command:CanExecute(parameter)
    return self._canExecute(parameter)
end

-- ========================================
-- Main ViewModel
-- ========================================

MainViewModel = {}
MainViewModel.__index = MainViewModel
setmetatable(MainViewModel, ViewModel)

function MainViewModel.new()
    local self = setmetatable(ViewModel.new(), MainViewModel)
    
    -- Properties
    self.Title = "LuaUI MVVM Demo"
    self.Description = "XML View + Lua ViewModel"
    self.Counter = 0
    self.IsFeatureEnabled = false
    self.FeatureStatusText = "Feature is currently disabled"
    self.ItemCount = 2
    self.Items = {
        { Name = "Item A", Value = 100 },
        { Name = "Item B", Value = 200 }
    }
    self.Status = "Ready"
    
    -- Commands
    self.IncrementCommand = function()
        self.Counter = self.Counter + 1
        self:UpdateStatus()
        print("Counter incremented to: " .. self.Counter)
    end
    
    self.DecrementCommand = function()
        self.Counter = self.Counter - 1
        self:UpdateStatus()
    end
    
    self.ToggleCommand = function()
        self.IsFeatureEnabled = not self.IsFeatureEnabled
        if self.IsFeatureEnabled then
            self.FeatureStatusText = "Feature is currently enabled"
        else
            self.FeatureStatusText = "Feature is currently disabled"
        end
        self:UpdateStatus()
        print("Feature toggled: " .. tostring(self.IsFeatureEnabled))
    end
    
    self.AddItemCommand = function()
        local count = #self.Items + 1
        table.insert(self.Items, { Name = "Item " .. count, Value = math.random(10, 99) })
        self.ItemCount = #self.Items
        self:UpdateStatus()
    end
    
    self.ClearItemsCommand = function()
        self.Items = {}
        self.ItemCount = 0
        self:UpdateStatus()
    end
    
    self.ResetCommand = function()
        self.Counter = 0
        self.IsFeatureEnabled = false
        self.FeatureStatusText = "Feature is currently disabled"
        self.Items = {
            { Name = "Item A", Value = 100 },
            { Name = "Item B", Value = 200 }
        }
        self.ItemCount = 2
        self:UpdateStatus()
        print("Reset completed")
    end
    
    return self
end

function MainViewModel:UpdateStatus()
    self.Status = string.format("Counter: %d | Items: %d | Feature: %s",
        self.Counter,
        self.ItemCount,
        self.IsFeatureEnabled and "ON" or "OFF"
    )
end

-- Create global instance
_G.ViewModelInstance = MainViewModel.new()

print("ViewModel loaded")
