-- LuaUI MVVM Demo - ViewModel
-- 
-- 使用方式：
-- 1. 定义属性（普通 table 字段）
-- 2. 定义命令（函数，以 Command 结尾自动绑定）
-- 3. 修改属性后调用 Notify("PropertyName") 触发 UI 更新

-- 启用日志
Log.info("[ViewModel] Initializing...")

-- 检查 Notify 函数是否存在
if type(Notify) ~= "function" then
    Log.error("[ViewModel] CRITICAL: Notify function is not available!")
else
    Log.info("[ViewModel] Notify function is available")
end

-- 创建 ViewModel
local ViewModel = {}

-- ============================================================================
-- 属性（绑定到 XML 的 {Binding Property}）
-- ============================================================================

ViewModel.Title = "LuaUI MVVM Demo"
ViewModel.Description = "Native Lua ViewModel with auto-binding"
ViewModel.Counter = 0
ViewModel.IsFeatureEnabled = false
ViewModel.FeatureStatusText = "Feature is OFF"  -- 计算属性，需要手动维护
ViewModel.Status = "Ready"

-- 更新 FeatureStatusText 计算属性
function ViewModel:UpdateFeatureStatus()
    self.FeatureStatusText = self.IsFeatureEnabled and "Feature is ON" or "Feature is OFF"
end

-- ============================================================================
-- 命令（以 Command 结尾自动绑定到 XML 的 Command="XXX"）
-- ============================================================================

function ViewModel:IncrementCommand()
    Log.info("[ViewModel] IncrementCommand called")
    self.Counter = self.Counter + 1
    self:UpdateStatus()
    
    -- 安全调用 Notify
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
    self:UpdateFeatureStatus()  -- 更新计算属性
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
    self:UpdateFeatureStatus()  -- 更新计算属性
    
    if type(Notify) == "function" then
        Notify("IsFeatureEnabled")
        Notify("FeatureStatusText")
    end
    
    Log.infof("Feature toggled: %s", self.IsFeatureEnabled and "ON" or "OFF")
end

-- ============================================================================
-- Items 相关
-- ============================================================================

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

-- ============================================================================
-- 辅助方法
-- ============================================================================

function ViewModel:UpdateStatus()
    self.Status = string.format("Counter: %d | Feature: %s", 
        self.Counter, 
        self.IsFeatureEnabled and "ON" or "OFF")
    
    if type(Notify) == "function" then
        Notify("Status")
    end
end

-- ============================================================================
-- 注册到全局（框架通过 viewModelName 参数查找，默认为 "ViewModelInstance"）
-- ============================================================================

_G.ViewModelInstance = ViewModel

Log.info("[ViewModel] Ready")
