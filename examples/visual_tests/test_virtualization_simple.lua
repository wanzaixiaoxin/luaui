--[[
    虚拟化性能测试 - 简化版本
    测试 ListBox 虚拟化功能
]]--

Log.info("=== Virtualization Test ===")

-- 创建主窗口
local window = Window.new()
window:setTitle("Virtualization Test - 10,000 Items")
window:setSize(800, 600)

-- 创建主面板
local panel = StackPanel.new()
panel:setOrientation("Vertical")
window:setContent(panel)

-- 标题
local title = TextBlock.new()
title:setText("ListBox Virtualization Test")
title:setFontSize(24)
panel:addChild(title)

-- 状态文本
local statusText = TextBlock.new()
statusText:setText("Status: Ready")
statusText:setFontSize(14)
panel:addChild(statusText)

-- 按钮面板
local buttonPanel = StackPanel.new()
buttonPanel:setOrientation("Horizontal")
buttonPanel:setSpacing(10)
panel:addChild(buttonPanel)

-- 创建 ListBox（使用虚拟化）
local listBox = ListBox.new()
listBox:setItemHeight(30)
listBox:setWidth(700)
listBox:setHeight(400)
panel:addChild(listBox)

-- 生成 10,000 条数据
local itemCount = 10000
local dataSource = {}
for i = 1, itemCount do
    dataSource[i] = "Item " .. i .. " - " .. string.rep("Data ", math.random(1, 5))
end

-- 设置数据源（启用虚拟化）
local startTime = os.clock()
listBox:setDataSource(itemCount, function(index)
    return dataSource[index + 1]  -- Lua 是 1-based，但 C++ 是 0-based
end)
local endTime = os.clock()

Log.infof("Data source set in %.2f ms", (endTime - startTime) * 1000)
statusText:setText(string.format("Loaded %d items in %.2f ms", itemCount, (endTime - startTime) * 1000))

-- 添加控制按钮
local test1kButton = Button.new()
test1kButton:setText("Load 1,000")
test1kButton:setWidth(100)
test1kButton:setHeight(30)
test1kButton:onClick(function()
    itemCount = 1000
    dataSource = {}
    for i = 1, itemCount do
        dataSource[i] = "Item " .. i .. " - Small Dataset"
    end
    
    local start = os.clock()
    listBox:setDataSource(itemCount, function(index)
        return dataSource[index + 1]
    end)
    local elapsed = (os.clock() - start) * 1000
    
    Log.infof("Loaded 1,000 items in %.2f ms", elapsed)
    statusText:setText(string.format("Loaded 1,000 items in %.2f ms", elapsed))
end)
buttonPanel:addChild(test1kButton)

local test10kButton = Button.new()
test10kButton:setText("Load 10,000")
test10kButton:setWidth(100)
test10kButton:setHeight(30)
test10kButton:onClick(function()
    itemCount = 10000
    dataSource = {}
    for i = 1, itemCount do
        dataSource[i] = "Item " .. i .. " - " .. string.rep("X", math.random(10, 50))
    end
    
    local start = os.clock()
    listBox:setDataSource(itemCount, function(index)
        return dataSource[index + 1]
    end)
    local elapsed = (os.clock() - start) * 1000
    
    Log.infof("Loaded 10,000 items in %.2f ms", elapsed)
    statusText:setText(string.format("Loaded 10,000 items in %.2f ms", elapsed))
end)
buttonPanel:addChild(test10kButton)

local test100kButton = Button.new()
test100kButton:setText("Load 100,000")
test100kButton:setWidth(100)
test100kButton:setHeight(30)
test100kButton:onClick(function()
    itemCount = 100000
    dataSource = {}
    for i = 1, itemCount do
        dataSource[i] = "Item " .. i .. " - Large Dataset Test"
    end
    
    local start = os.clock()
    listBox:setDataSource(itemCount, function(index)
        return dataSource[index + 1]
    end)
    local elapsed = (os.clock() - start) * 1000
    
    Log.infof("Loaded 100,000 items in %.2f ms", elapsed)
    statusText:setText(string.format("Loaded 100,000 items in %.2f ms", elapsed))
end)
buttonPanel:addChild(test100kButton)

-- 选择变化事件
listBox:onSelectionChanged(function(index)
    Log.infof("Selected index: %d", index)
    if index >= 0 and index < itemCount then
        statusText:setText(string.format("Selected: %s", dataSource[index + 1]))
    end
end)

Log.info("Virtualization test UI created successfully")

-- 运行窗口
window:show()

-- 保持脚本运行
return true
