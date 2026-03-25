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
-- Items 相关 - 列表控件演示
-- ============================================================================
-- 使用 AutoViewModel 提供的数组操作方法：AddItem, RemoveItem, ClearItems
-- 这些方法会自动触发属性变更通知
-- 初始添加一些演示数据
ViewModel.Items = {
    { Name = "Apple", Category = "Fruit", Value = 10, Stock = 32 },
    { Name = "Banana", Category = "Fruit", Value = 25, Stock = 18 },
    { Name = "Cherry", Category = "Fruit", Value = 50, Stock = 64 }
}
ViewModel.ItemCount = "Items: 3"
ViewModel.SelectedIndex = -1  -- -1 表示无选中项

-- 计算属性：显示选中项的详细信息
ViewModel:DefineComputed("SelectedItemText",
    {"Items", "SelectedIndex"},
    function(self)
        local idx = self.SelectedIndex
        if idx and idx >= 0 and idx < #self.Items then
            local item = self.Items[idx + 1]  -- Lua 索引从 1 开始
            return string.format("Selected: %s (Value: %d)", item.Name, item.Value)
        end
        return "No item selected"
    end
)

function ViewModel:UpdateItemCount()
    local count = #self.Items
    self.ItemCount = string.format("Items: %d", count)
    -- 如果选中项超出范围，重置选择
    if self.SelectedIndex >= count then
        self.SelectedIndex = count - 1
    end
    Log.infof("[ViewModel] ItemCount updated: %d", count)
end

function ViewModel:AddItemCommand()
    local categories = { "Fruit", "Snack", "Drink", "Office" }
    local newItem = {
        Name = "Item " .. tostring(#self.Items + 1),
        Category = categories[(#self.Items % #categories) + 1],
        Value = math.random(100),
        Stock = math.random(5, 80)
    }
    -- 使用 AutoViewModel 提供的 AddItem 方法（自动触发通知）
    self:AddItem("Items", newItem)
    self:UpdateItemCount()
    self:UpdateStatus()
end

function ViewModel:RemoveItemCommand()
    Log.info("[ViewModel] RemoveItemCommand called")
    local idx = self.SelectedIndex
    if idx and idx >= 0 and idx < #self.Items then
        -- 使用 AutoViewModel 提供的 RemoveItem 方法（自动触发通知）
        local removed = self:RemoveItem("Items", idx + 1)  -- Lua 索引从 1 开始
        -- 调整选中索引
        if idx >= #self.Items then
            self.SelectedIndex = #self.Items - 1
        end
        self:UpdateItemCount()
        self:UpdateStatus()
        Log.infof("Removed item: %s", removed and removed.Name or "unknown")
    else
        Log.warn("[ViewModel] No item selected to remove")
    end
end

function ViewModel:ClearItemsCommand()
    Log.info("[ViewModel] ClearItemsCommand called")
    self.SelectedIndex = -1
    -- 使用 AutoViewModel 提供的 ClearItems 方法（自动触发通知）
    self:ClearItems("Items")
    self:UpdateItemCount()
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
