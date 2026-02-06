-- Hello World Lua Script for LuaUI

-- 设置日志级别
local logSuccess, logResult = pcall(function()
    Log.setLevel("DEBUG")
end)

if not logSuccess then
    print("Failed to set log level: " .. tostring(logResult))
else
    print("Log level set to DEBUG successfully")
end

-- 全局变量
local clickCount = 0

-- 按钮点击事件处理
function onHelloClick()
    -- 添加异常处理
    local success, result = pcall(function()
        clickCount = clickCount + 1

        Log.info("Button clicked, count: " .. clickCount)

        local messages = {
            "Hello, LuaUI!",
            "Great job!",
            "Keep clicking!",
            "You clicked " .. clickCount .. " times!",
            "Lua is awesome!"
        }

        -- 使用基于控件ID的函数调用方式
        Log.debug("Attempting to update label text...")
        
        -- 先检查控件是否存在
        local control = UI:getControl("lblMessage")
        if control then
            Log.debug("Found lblMessage control")
            local setSuccess = UI:setProperty("lblMessage", "text", messages[math.min(clickCount, #messages)])
            
            if setSuccess then
                Log.debug("Updated message label text successfully")
            else
                Log.warn("Failed to update message label text")
            end
        else
            Log.warn("lblMessage control not found in control map")
        end
        
        return true
    end)
    
    if not success then
        Log.error("Error in onHelloClick: " .. tostring(result))
        return false
    end
    
    return result
end

-- 退出按钮点击事件
function onExitClick()
    Log.warn("User clicked exit button")
    UI:exit()
end

-- 窗口初始化
function onInit()
    print("onInit called")
    Log.info("Hello World application initialized")
    Log.debug("Lua environment ready")

    -- 检查所有控件是否存在
    Log.debug("Checking control registration...")
    local controls = {"btnHello", "btnExit", "lblMessage", "lblTitle", "mainWindow"}
    for i, controlId in ipairs(controls) do
        local control = UI:getControl(controlId)
        if control then
            Log.debug("Control found: " .. controlId)
        else
            Log.warn("Control NOT found: " .. controlId)
        end
    end

    -- 绑定按钮事件
    local result1 = UI:bindEvent("btnHello", "onClick", onHelloClick)
    local result2 = UI:bindEvent("btnExit", "onClick", onExitClick)
    
    Log.info("UI:bindEvent btnHello result: " .. tostring(result1))
    Log.info("UI:bindEvent btnExit result: " .. tostring(result2))

    Log.info("Events bound successfully")
end

-- 窗口关闭
function onClose()
    Log.info("Hello World application closing")
end
