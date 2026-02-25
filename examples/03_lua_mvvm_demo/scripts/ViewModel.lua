-- LuaUI MVVM Demo - ViewModel
-- 纯业务代码，框架(AutoViewModel等)由C++层自动提供

Log.info("[ViewModel] Initializing...")

-- ============================================================================
-- 创建 ViewModel（使用 C++ 层提供的 AutoViewModel）
-- ============================================================================
local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- ============================================================================
-- 属性定义（自动触发 UI 更新）
-- ============================================================================
ViewModel.Title = "LuaUI MVVM Demo"
ViewModel.Description = "Clean ViewModel - Framework handles UI updates"
ViewModel.Counter = 0
ViewModel.IsFeatureEnabled = false
ViewModel.Status = "Ready"

-- 计算属性：自动根据 IsFeatureEnabled 计算
ViewModel:DefineComputed("FeatureStatusText", 
    {"IsFeatureEnabled"},
    function(self)
        return self.IsFeatureEnabled and "Feature is ON" or "Feature is OFF"
    end
)

-- ============================================================================
-- 命令（以 Command 结尾自动绑定到 XML Click="XXXCommand"）
-- ============================================================================
function ViewModel:IncrementCommand()
    Log.info("[ViewModel] IncrementCommand called")
    self.Counter = self.Counter + 1
    self:UpdateStatus()
    Log.infof("Counter incremented to: %d", self.Counter)
end

function ViewModel:DecrementCommand()
    Log.info("[ViewModel] DecrementCommand called")
    self.Counter = self.Counter - 1
    self:UpdateStatus()
    Log.infof("Counter decremented to: %d", self.Counter)
end

function ViewModel:ResetCommand()
    Log.info("[ViewModel] ResetCommand called")
    -- 批量更新：多个属性变更只触发一次 UI 刷新
    self:BeginBatch()
    self.Counter = 0
    self.IsFeatureEnabled = false
    -- FeatureStatusText 作为计算属性会自动更新
    self:EndBatch()
    self:UpdateStatus()
    Log.info("Reset completed")
end

function ViewModel:ToggleCommand()
    Log.info("[ViewModel] ToggleCommand called")
    self.IsFeatureEnabled = not self.IsFeatureEnabled
    self:UpdateStatus()
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
    table.insert(self.Items, { 
        Name = "Item " .. tostring(#self.Items + 1), 
        Value = math.random(100) 
    })
    self:UpdateStatus()
    Log.infof("Added item, total: %d", #self.Items)
end

function ViewModel:ClearItemsCommand()
    Log.info("[ViewModel] ClearItemsCommand called")
    self.Items = {}
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
end

-- ============================================================================
-- 注册到全局（框架查找）
-- ============================================================================
_G.ViewModelInstance = ViewModel

Log.info("[ViewModel] Ready - Clean ViewModel with framework-provided AutoViewModel")
