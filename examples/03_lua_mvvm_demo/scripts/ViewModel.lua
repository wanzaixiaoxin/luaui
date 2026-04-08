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

-- ============================================================================
-- Button 新功能测试属性
-- ============================================================================

-- 3. 动态禁用测试
ViewModel.IsBtnDisabled = false
ViewModel:DefineComputed("ToggleBtnText",
    {"IsBtnDisabled"},
    function(self)
        return self.IsBtnDisabled and "点击启用" or "点击禁用"
    end
)
ViewModel:DefineComputed("DisableStatusText",
    {"IsBtnDisabled"},
    function(self)
        return self.IsBtnDisabled and "当前: 已禁用" or "当前: 已启用"
    end
)

-- 5. MVVM 绑定：按钮文本
ViewModel.BtnLabelIndex = 1
ViewModel.BtnLabel = "Hello"
ViewModel.BtnLabelOptions = { "Hello", "World", "LuaUI", "Button", "MVVM" }

-- 7. 点击计数
ViewModel.BtnClickCount = 0
ViewModel:DefineComputed("BtnClickCountText",
    {"BtnClickCount"},
    function(self)
        return string.format("已点击 %d 次", self.BtnClickCount)
    end
)

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
    self:BeginBatch()
    self.Counter = 0
    self.IsFeatureEnabled = false
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
-- Button 新功能测试命令
-- ============================================================================

-- 3. 动态禁用切换
function ViewModel:ToggleDisableCommand()
    self.IsBtnDisabled = not self.IsBtnDisabled
    Log.infof("[ViewModel] Button disabled: %s", self.IsBtnDisabled and "true" or "false")
end

-- 5. MVVM 绑定：切换按钮文本
function ViewModel:ChangeLabelCommand()
    self.BtnLabelIndex = (self.BtnLabelIndex % #self.BtnLabelOptions) + 1
    self.BtnLabel = self.BtnLabelOptions[self.BtnLabelIndex]
    Log.infof("[ViewModel] BtnLabel changed to: %s", self.BtnLabel)
end

-- 7. 点击计数
function ViewModel:BtnClickCountCommand()
    self.BtnClickCount = self.BtnClickCount + 1
    Log.infof("[ViewModel] Button clicked: %d times", self.BtnClickCount)
end

-- ============================================================================
-- Items 相关 - 列表控件演示
-- ============================================================================
ViewModel.Items = {
    { Name = "Apple", Category = "Fruit", Value = 10, Stock = 32 },
    { Name = "Banana", Category = "Fruit", Value = 25, Stock = 18 },
    { Name = "Cherry", Category = "Fruit", Value = 50, Stock = 64 }
}
ViewModel.ItemCount = "Items: 3"
ViewModel.SelectedIndex = -1

ViewModel:DefineComputed("SelectedItemText",
    {"Items", "SelectedIndex"},
    function(self)
        local idx = self.SelectedIndex
        if idx and idx >= 0 and idx < #self.Items then
            local item = self.Items[idx + 1]
            return string.format("Selected: %s (Value: %d)", item.Name, item.Value)
        end
        return "No item selected"
    end
)

function ViewModel:UpdateItemCount()
    local count = #self.Items
    self.ItemCount = string.format("Items: %d", count)
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
    self:AddItem("Items", newItem)
    self:UpdateItemCount()
    self:UpdateStatus()
end

function ViewModel:RemoveItemCommand()
    Log.info("[ViewModel] RemoveItemCommand called")
    local idx = self.SelectedIndex
    if idx and idx >= 0 and idx < #self.Items then
        local removed = self:RemoveItem("Items", idx + 1)
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
    self:ClearItems("Items")
    self:UpdateItemCount()
    self:UpdateStatus()
    Log.info("Items cleared")
end

-- ============================================================================
-- 辅助方法
-- ============================================================================
function ViewModel:UpdateStatus()
    self.Status = string.format("Counter: %d | Feature: %s | Clicks: %d",
        self.Counter,
        self.IsFeatureEnabled and "ON" or "OFF",
        self.BtnClickCount)
end

-- ============================================================================
-- 注册到全局（框架查找）
-- ============================================================================
_G.ViewModelInstance = ViewModel

Log.info("[ViewModel] Ready - Clean ViewModel with framework-provided AutoViewModel")
