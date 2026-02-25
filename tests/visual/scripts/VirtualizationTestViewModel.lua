-- VirtualizationTest ViewModel - 10,000+ 行性能测试

Log.info("[VirtualizationTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- =============================================================================
-- Properties
-- =============================================================================

-- 测试数据量
ViewModel.ItemCount = 10000
ViewModel.DisplayCount = 0
ViewModel.StatusText = "Ready"
ViewModel.LoadTime = "0 ms"

-- 数据生成
function ViewModel:GenerateItemText(index)
    -- 生成股票行情样式的数据
    local stockCode = string.format("%06d", index % 999999)
    local stockName = "Stock " .. index
    local price = 10 + math.random() * 190  -- 10-200 随机价格
    local change = (math.random() - 0.5) * 10  -- -5% ~ +5% 涨跌幅
    
    return string.format("%s  %-12s  %8.2f  %+6.2f%%", 
        stockCode, stockName, price, change)
end

-- 数据获取回调（供 C++ 调用）
function ViewModel:GetItemText(index)
    if index < 0 or index >= self.ItemCount then
        return ""
    end
    return self:GenerateItemText(index)
end

-- =============================================================================
-- Commands
-- =============================================================================

-- 生成 1,000 项
function ViewModel:Load1000Command()
    Log.info("[VirtualizationTest] Loading 1,000 items")
    self:LoadItems(1000)
end

-- 生成 10,000 项
function ViewModel:Load10000Command()
    Log.info("[VirtualizationTest] Loading 10,000 items")
    self:LoadItems(10000)
end

-- 生成 100,000 项
function ViewModel:Load100000Command()
    Log.info("[VirtualizationTest] Loading 100,000 items")
    self:LoadItems(100000)
end

-- 清空
function ViewModel:ClearCommand()
    Log.info("[VirtualizationTest] Clearing items")
    self.ItemCount = 0
    self.DisplayCount = 0
    self.StatusText = "Cleared"
    self.LoadTime = "0 ms"
    
    -- 通知更新
    self:_notifyChange("ItemCount")
end

-- 滚动到指定位置
function ViewModel:ScrollToEndCommand()
    Log.info("[VirtualizationTest] Scroll to end")
    self.StatusText = "Scrolling to item " .. (self.ItemCount - 1)
end

function ViewModel:ScrollToMiddleCommand()
    Log.info("[VirtualizationTest] Scroll to middle")
    local middle = math.floor(self.ItemCount / 2)
    self.StatusText = "Scrolling to item " .. middle
end

-- =============================================================================
-- Methods
-- =============================================================================

function ViewModel:LoadItems(count)
    self.StatusText = "Loading " .. count .. " items..."
    
    local startTime = os.clock()
    
    -- 设置数据量（虚拟化模式下不会立即创建所有项）
    self.ItemCount = count
    self.DisplayCount = count
    
    local endTime = os.clock()
    local loadTime = (endTime - startTime) * 1000
    
    self.LoadTime = string.format("%.2f ms", loadTime)
    self.StatusText = string.format("Loaded %d items in %s (virtualized)", 
        count, self.LoadTime)
    
    Log.infof("[VirtualizationTest] Loaded %d items in %.2f ms", count, loadTime)
    
    -- 通知属性变更
    self:_notifyChange("ItemCount")
    self:_notifyChange("DisplayCount")
end

-- =============================================================================
-- Initialization
-- =============================================================================

-- 初始加载 10,000 项
ViewModel:LoadItems(10000)

-- Register globally
_G.VirtualizationTestViewModel = ViewModel

Log.info("[VirtualizationTestViewModel] Ready")
