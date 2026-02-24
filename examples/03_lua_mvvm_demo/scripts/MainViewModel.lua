-- MainViewModel.lua
-- MVVM Demo ViewModel
-- Demonstrates: Properties, Commands, ObservableCollection, INotifyPropertyChanged

-- Create ViewModel instance
function CreateViewModel()
    local self = setmetatable({}, ViewModel)
    
    -- ========================================
    -- Properties (with change notification)
    -- ========================================
    self.Title = "LuaUI MVVM Demo"
    self.Description = "Demonstrating MVVM pattern with Lua ViewModel"
    
    -- Counter property
    self._counter = 0
    self.Counter = 0
    
    -- Input property
    self.InputText = ""
    
    -- Feature toggle
    self._isFeatureEnabled = false
    self.IsFeatureEnabled = false
    
    -- Computed property
    self.Status = "Ready"
    
    -- ========================================
    -- ObservableCollection Example
    -- ========================================
    self.Items = ObservableCollection.new()
    
    -- Subscribe to collection changes
    self.Items:Subscribe(function(action, item, index)
        LogInfo("Collection changed: " .. action .. " at index " .. index)
        self:UpdateStatus()
    end)
    
    -- Add some initial items
    self.Items:Add({ Name = "Item 1", Value = 10 })
    self.Items:Add({ Name = "Item 2", Value = 20 })
    self.Items:Add({ Name = "Item 3", Value = 30 })
    
    -- ========================================
    -- Commands
    -- ========================================
    
    -- Increment command
    self.IncrementCommand = Command.new(function(param)
        self._counter = self._counter + 1
        self.Counter = self._counter
        self:PropertyChanged("Counter")
        self:UpdateStatus()
        LogInfo("Counter incremented to: " .. self.Counter)
    end)
    
    -- Decrement command
    self.DecrementCommand = Command.new(function(param)
        self._counter = self._counter - 1
        self.Counter = self._counter
        self:PropertyChanged("Counter")
        self:UpdateStatus()
    end)
    
    -- Toggle feature command
    self.ToggleCommand = Command.new(function(param)
        self._isFeatureEnabled = not self._isFeatureEnabled
        self.IsFeatureEnabled = self._isFeatureEnabled
        self:PropertyChanged("IsFeatureEnabled")
        self:UpdateStatus()
        LogInfo("Feature toggled: " .. tostring(self.IsFeatureEnabled))
    end)
    
    -- Add item command
    self.AddItemCommand = Command.new(function(param)
        local index = self.Items:Count() + 1
        self.Items:Add({ 
            Name = "Item " .. index, 
            Value = math.random(1, 100) 
        })
        self:UpdateStatus()
    end)
    
    -- Clear items command
    self.ClearItemsCommand = Command.new(function(param)
        self.Items:Clear()
        self:UpdateStatus()
    end, function(param)
        -- CanExecute: only if has items
        return self.Items:Count() > 0
    end)
    
    -- Reset command
    self.ResetCommand = Command.new(function(param)
        self._counter = 0
        self.Counter = 0
        self._isFeatureEnabled = false
        self.IsFeatureEnabled = false
        self.InputText = ""
        
        self:PropertyChanged("Counter")
        self:PropertyChanged("IsFeatureEnabled")
        self:PropertyChanged("InputText")
        
        self.Items:Clear()
        self.Items:Add({ Name = "Item 1", Value = 10 })
        
        self:UpdateStatus()
        LogInfo("ViewModel reset")
    end)
    
    -- ========================================
    -- Methods
    -- ========================================
    
    function self:UpdateStatus()
        local count = self.Items:Count()
        if self._isFeatureEnabled then
            self.Status = "Active - Counter: " .. self.Counter .. ", Items: " .. count
        else
            self.Status = "Standby - Counter: " .. self.Counter .. ", Items: " .. count
        end
        self:PropertyChanged("Status")
    end
    
    function self:OnInputChanged(newValue)
        self.InputText = newValue
        self:PropertyChanged("InputText")
        self:UpdateStatus()
    end
    
    -- Initialize status
    self:UpdateStatus()
    
    return self
end

-- ========================================
-- Extension Methods for Data Binding
-- ========================================

-- Format collection for display
function FormatCollection(collection)
    local result = {}
    for i = 1, collection:Count() do
        local item = collection:Get(i)
        table.insert(result, item.Name .. "=" .. item.Value)
    end
    return table.concat(result, ", ")
end

-- Validation helper
function ValidateInput(text)
    if not text or text == "" then
        return false, "Input cannot be empty"
    end
    if #text < 3 then
        return false, "Input must be at least 3 characters"
    end
    return true, nil
end
