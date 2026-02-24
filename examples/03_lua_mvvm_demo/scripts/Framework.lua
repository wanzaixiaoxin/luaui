-- Framework.lua
-- MVVM Framework for LuaUI
-- Provides: ViewModel base, ObservableCollection, Command pattern

-- ========================================
-- ViewModel Base Class
-- ========================================
ViewModel = {}
ViewModel.__index = ViewModel

function ViewModel.new()
    local self = setmetatable({}, ViewModel)
    self._listeners = {}
    return self
end

-- Notify property changed (triggers UI update)
function ViewModel:PropertyChanged(propertyName)
    if self._listeners[propertyName] then
        for _, callback in ipairs(self._listeners[propertyName]) do
            callback(self[propertyName])
        end
    end
end

-- Subscribe to property changes
function ViewModel:Subscribe(propertyName, callback)
    if not self._listeners[propertyName] then
        self._listeners[propertyName] = {}
    end
    table.insert(self._listeners[propertyName], callback)
end

-- ========================================
-- ObservableCollection
-- For binding lists to DataGrid, ListBox, etc.
-- ========================================
ObservableCollection = {}
ObservableCollection.__index = ObservableCollection

function ObservableCollection.new()
    local self = setmetatable({}, ObservableCollection)
    self._items = {}
    self._listeners = {}
    return self
end

function ObservableCollection:Add(item)
    table.insert(self._items, item)
    self:NotifyChanged("Added", item, #self._items)
end

function ObservableCollection:Remove(item)
    for i, v in ipairs(self._items) do
        if v == item then
            table.remove(self._items, i)
            self:NotifyChanged("Removed", item, i)
            return true
        end
    end
    return false
end

function ObservableCollection:RemoveAt(index)
    if index >= 1 and index <= #self._items then
        local item = self._items[index]
        table.remove(self._items, index)
        self:NotifyChanged("Removed", item, index)
        return item
    end
    return nil
end

function ObservableCollection:Clear()
    self._items = {}
    self:NotifyChanged("Cleared", nil, 0)
end

function ObservableCollection:Get(index)
    return self._items[index]
end

function ObservableCollection:Count()
    return #self._items
end

function ObservableCollection:GetAll()
    return self._items
end

function ObservableCollection:Subscribe(callback)
    table.insert(self._listeners, callback)
end

function ObservableCollection:NotifyChanged(action, item, index)
    for _, callback in ipairs(self._listeners) do
        callback(action, item, index)
    end
end

-- ========================================
-- Command Pattern
-- For button clicks and actions
-- ========================================
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
-- UI Helpers
-- ========================================

-- Create a control (placeholder - real binding would create actual controls)
function CreateControl(typeName, name)
    return {
        Type = typeName,
        Name = name,
        Properties = {},
        Children = {},
        SetProperty = function(self, prop, value)
            self.Properties[prop] = value
        end,
        GetProperty = function(self, prop)
            return self.Properties[prop]
        end,
        AddChild = function(self, child)
            table.insert(self.Children, child)
        end
    }
end

-- ========================================
-- Binding Helpers
-- ========================================

-- Bind a control property to ViewModel property
function Bind(control, controlProperty, viewModel, viewModelProperty, mode)
    mode = mode or "OneWay"
    
    -- Initial value
    control:SetProperty(controlProperty, viewModel[viewModelProperty])
    
    -- Subscribe to changes
    if viewModel.Subscribe then
        viewModel:Subscribe(viewModelProperty, function(newValue)
            control:SetProperty(controlProperty, newValue)
        end)
    end
    
    -- Two-way: control changes update ViewModel
    if mode == "TwoWay" then
        -- In real implementation, hook control's change event
    end
end

-- Bind command to button click
function BindCommand(button, viewModel, commandName)
    -- In real implementation, set button's Click handler
    button.OnClick = function()
        local cmd = viewModel[commandName]
        if cmd and type(cmd) == "table" and cmd.Execute then
            cmd:Execute()
        elseif type(cmd) == "function" then
            cmd(viewModel)
        end
    end
end

-- Use print if Log is not available
if Log and Log.info then
    Log.info("Framework loaded")
else
    print("Framework loaded")
end
