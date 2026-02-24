-- LuaUI MVVM Demo - ViewModel
-- 
-- 使用方式：
-- 1. 定义属性（普通 table 字段）
-- 2. 定义命令（函数，以 Command 结尾自动绑定）
-- 3. 修改属性后调用 Notify("PropertyName") 触发 UI 更新

-- 启用日志
Log.info("[ViewModel] Initializing...")

-- 创建 ViewModel
local ViewModel = {}

-- ============================================================================
-- 属性（绑定到 XML 的 {Binding Property}）
-- ============================================================================

ViewModel.Title = "LuaUI MVVM Demo"
ViewModel.Description = "Native Lua ViewModel with auto-binding"
ViewModel.Counter = 0
ViewModel.IsFeatureEnabled = false
ViewModel.Status = "Ready"

-- 计算属性
function ViewModel:GetFeatureStatusText()
    return self.IsFeatureEnabled and "Feature is ON" or "Feature is OFF"
end

-- ============================================================================
-- 命令（以 Command 结尾自动绑定到 XML 的 Command="XXX"）
-- ============================================================================

function ViewModel:IncrementCommand()
    self.Counter = self.Counter + 1
    self:UpdateStatus()
    Notify("Counter")  -- 触发 UI 更新
    Log.infof("Counter incremented to: %d", self.Counter)
end

function ViewModel:DecrementCommand()
    self.Counter = self.Counter - 1
    self:UpdateStatus()
    Notify("Counter")
    Log.infof("Counter decremented to: %d", self.Counter)
end

function ViewModel:ResetCommand()
    self.Counter = 0
    self:UpdateStatus()
    Notify("Counter")
    Log.info("Counter reset")
end

function ViewModel:ToggleCommand()
    self.IsFeatureEnabled = not self.IsFeatureEnabled
    Notify("IsFeatureEnabled")
    Notify("FeatureStatusText")  -- 计算属性也需要通知
    Log.infof("Feature toggled: %s", self.IsFeatureEnabled and "ON" or "OFF")
end

-- ============================================================================
-- 辅助方法
-- ============================================================================

function ViewModel:UpdateStatus()
    self.Status = string.format("Counter: %d | Feature: %s", 
        self.Counter, 
        self.IsFeatureEnabled and "ON" or "OFF")
    Notify("Status")
end

-- ============================================================================
-- 注册到全局（框架通过 viewModelName 参数查找，默认为 "ViewModelInstance"）
-- ============================================================================

_G.ViewModelInstance = ViewModel

Log.info("[ViewModel] Ready")
