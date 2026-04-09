-- LuaUI MVVM Demo - ViewModel
-- 纯业务代码，框架(AutoViewModel等)由C++层自动提供

Log.info("[ViewModel] Initializing...")

local function char_count(value)
    if not value or value == "" then
        return 0
    end

    local ok, len = pcall(utf8.len, value)
    if ok and len then
        return len
    end

    return #value
end

local function password_strength(value)
    local length = char_count(value)
    local score = 0

    if length >= 8 then score = score + 1 end
    if value:match("%u") then score = score + 1 end
    if value:match("%l") then score = score + 1 end
    if value:match("%d") then score = score + 1 end
    if value:match("[%p_]") then score = score + 1 end

    if score >= 5 then return "strong" end
    if score >= 3 then return "medium" end
    if length > 0 then return "weak" end
    return "empty"
end

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
-- TextBox 测试属性
-- ============================================================================
ViewModel.LiveText = "Edit either LiveTextBox or MirrorTextBox to verify TwoWay synchronization."
ViewModel.LimitedText = ""
ViewModel.PasswordText = ""
ViewModel.ReadOnlyText = "This TextBox is read-only. Use the buttons below to replace its value from Lua."
ViewModel.ScrollText = ""

ViewModel:DefineComputed("LiveTextSummary",
    {"LiveText"},
    function(self)
        return string.format("LiveText length: %d | Two TextBoxes share the same property.",
            char_count(self.LiveText))
    end
)

ViewModel:DefineComputed("LimitedTextSummary",
    {"LimitedText"},
    function(self)
        return string.format("LimitedText length: %d / 12 | Typing and paste should both clamp.",
            char_count(self.LimitedText))
    end
)

ViewModel:DefineComputed("PasswordSummary",
    {"PasswordText"},
    function(self)
        return string.format("Password length: %d | Strength: %s",
            char_count(self.PasswordText),
            password_strength(self.PasswordText))
    end
)

ViewModel:DefineComputed("ReadOnlySummary",
    {"ReadOnlyText"},
    function(self)
        return string.format("ReadOnly text length: %d | Focus, selection and copy should still work.",
            char_count(self.ReadOnlyText))
    end
)

ViewModel:DefineComputed("ScrollTextSummary",
    {"ScrollText"},
    function(self)
        return string.format("ScrollText length: %d | Use Home/End, Ctrl+Arrow and drag selection here.",
            char_count(self.ScrollText))
    end
)

ViewModel:DefineComputed("TextBoxStatus",
    {"LiveText", "LimitedText", "PasswordText", "ReadOnlyText", "ScrollText"},
    function(self)
        return string.format(
            "Status | Live=%d | Limited=%d | Password=%d | ReadOnly=%d | Scroll=%d",
            char_count(self.LiveText),
            char_count(self.LimitedText),
            char_count(self.PasswordText),
            char_count(self.ReadOnlyText),
            char_count(self.ScrollText))
    end
)

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
-- 主题切换
-- ============================================================================
ViewModel.CurrentTheme = "Light"

function ViewModel:ToggleThemeCommand()
    Log.info("[ViewModel] ToggleThemeCommand called")
    local newTheme = (self.CurrentTheme == "Light") and "Dark" or "Light"
    self.CurrentTheme = newTheme
    
    -- 调用 C++ 主题切换函数
    if Theme and Theme.SetCurrent then
        Theme.SetCurrent(newTheme)
    end
    
    Log.infof("Theme switched to: %s", newTheme)
end

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
-- TextBox 测试命令
-- ============================================================================
function ViewModel:LoadGreetingCommand()
    self.LiveText = "Hello from Lua ViewModel. Edit either TextBox to test TwoWay updates."
    self:UpdateStatus()
end

function ViewModel:LoadLongTextCommand()
    self.LiveText = "LuaUI_TextBox_Test__0123456789__ABCDEFGHIJKLMNOPQRSTUVWXYZ__cursor-navigation__selection__undo-redo__clipboard"
    self:UpdateStatus()
end

function ViewModel:LoadMixedTextCommand()
    self.LiveText = "LuaUI TextBox mixed input 你好 123 ABC ctrl+arrow home end"
    self:UpdateStatus()
end

function ViewModel:ClearLiveTextCommand()
    self.LiveText = ""
    self:UpdateStatus()
end

function ViewModel:FillLimitedSampleCommand()
    self.LimitedText = "AB12-CD34"
    self:UpdateStatus()
end

function ViewModel:ClearLimitedTextCommand()
    self.LimitedText = ""
    self:UpdateStatus()
end

function ViewModel:FillPasswordSampleCommand()
    self.PasswordText = "P@ssw0rd123"
    self:UpdateStatus()
end

function ViewModel:ClearPasswordCommand()
    self.PasswordText = ""
    self:UpdateStatus()
end

function ViewModel:LoadReadonlyShortCommand()
    self.ReadOnlyText = "Lua replaced the read-only TextBox with this short sample."
    self:UpdateStatus()
end

function ViewModel:LoadReadonlyLongCommand()
    self.ReadOnlyText = "Lua replaced the read-only TextBox with a longer sample so you can verify selection, copy and keyboard navigation without allowing direct edits."
    self:UpdateStatus()
end

function ViewModel:LoadScrollPathCommand()
    self.ScrollText = "D:\\workspace\\LuaUI\\examples\\03_lua_mvvm_demo\\logs\\2026\\04\\08\\TextBox\\super_long_file_name_for_horizontal_scrolling_validation.txt"
    self:UpdateStatus()
end

function ViewModel:LoadScrollCommandCommand()
    self.ScrollText = "--input=TextBox --mode=TwoWay --profile=debug --scenario=selection-navigation-undo-redo --locale=zh-CN"
    self:UpdateStatus()
end

function ViewModel:ClearScrollTextCommand()
    self.ScrollText = ""
    self:UpdateStatus()
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
    self.Status = string.format("Counter: %d | Feature: %s | Clicks: %d | LiveText: %d | Password: %d",
        self.Counter,
        self.IsFeatureEnabled and "ON" or "OFF",
        self.BtnClickCount,
        char_count(self.LiveText),
        char_count(self.PasswordText))
end

-- ============================================================================
-- 注册到全局（框架查找）
-- ============================================================================
ViewModel:UpdateStatus()
_G.ViewModelInstance = ViewModel

Log.info("[ViewModel] Ready - Clean ViewModel with framework-provided AutoViewModel")
