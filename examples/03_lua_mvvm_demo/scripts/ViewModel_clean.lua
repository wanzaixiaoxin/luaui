-- LuaUI MVVM Demo - ViewModel

Log.info("[ViewModel] Initializing...")

if type(Notify) ~= "function" then
    Log.error("[ViewModel] CRITICAL: Notify function is not available!")
else
    Log.info("[ViewModel] Notify function is available")
end

local ViewModel = {}

ViewModel.Title = "LuaUI MVVM Demo"
ViewModel.Description = "Native Lua ViewModel with auto-binding"
ViewModel.Counter = 0
ViewModel.IsFeatureEnabled = false
ViewModel.Status = "Ready"

function ViewModel:GetFeatureStatusText()
    return self.IsFeatureEnabled and "Feature is ON" or "Feature is OFF"
end

function ViewModel:IncrementCommand()
    Log.info("[ViewModel] IncrementCommand called")
    self.Counter = self.Counter + 1
    self:UpdateStatus()
    
    if type(Notify) == "function" then
        Log.info("[ViewModel] Calling Notify(Counter)...")
        Notify("Counter")
        Log.info("[ViewModel] Notify(Counter) completed")
    else
        Log.error("[ViewModel] Notify function not available!")
    end
    
    Log.infof("Counter incremented to: %d", self.Counter)
end

function ViewModel:DecrementCommand()
    Log.info("[ViewModel] DecrementCommand called")
    self.Counter = self.Counter - 1
    self:UpdateStatus()
    
    if type(Notify) == "function" then
        Notify("Counter")
    end
    
    Log.infof("Counter decremented to: %d", self.Counter)
end

function ViewModel:ResetCommand()
    Log.info("[ViewModel] ResetCommand called")
    self.Counter = 0
    self.IsFeatureEnabled = false
    self:UpdateStatus()
    
    if type(Notify) == "function" then
        Notify("Counter")
        Notify("IsFeatureEnabled")
        Notify("FeatureStatusText")
    end
    
    Log.info("Reset completed")
end

function ViewModel:ToggleCommand()
    Log.info("[ViewModel] ToggleCommand called")
    self.IsFeatureEnabled = not self.IsFeatureEnabled
    
    if type(Notify) == "function" then
        Notify("IsFeatureEnabled")
        Notify("FeatureStatusText")
    end
    
    Log.infof("Feature toggled: %s", self.IsFeatureEnabled and "ON" or "OFF")
end

ViewModel.Items = {}

function ViewModel:GetItemCount()
    return string.format("Items: %d", #self.Items)
end

function ViewModel:AddItemCommand()
    Log.info("[ViewModel] AddItemCommand called")
    table.insert(self.Items, { Name = "Item " .. tostring(#self.Items + 1), Value = math.random(100) })
    
    if type(Notify) == "function" then
        Notify("ItemCount")
    end
    
    self:UpdateStatus()
    Log.infof("Added item, total: %d", #self.Items)
end

function ViewModel:ClearItemsCommand()
    Log.info("[ViewModel] ClearItemsCommand called")
    self.Items = {}
    
    if type(Notify) == "function" then
        Notify("ItemCount")
    end
    
    self:UpdateStatus()
    Log.info("Items cleared")
end

function ViewModel:UpdateStatus()
    self.Status = string.format("Counter: %d | Feature: %s", 
        self.Counter, 
        self.IsFeatureEnabled and "ON" or "OFF")
    
    if type(Notify) == "function" then
        Notify("Status")
    end
end

_G.ViewModelInstance = ViewModel

Log.info("[ViewModel] Ready")
