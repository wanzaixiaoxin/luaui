-- Hello World Lua Script for LuaUI

-- 设置日志级别
Log.setLevel("DEBUG")

-- 全局变量
local clickCount = 0

-- 按钮点击事件处理
function onHelloClick()
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
    local success = UI:setProperty("lblMessage", "text", messages[math.min(clickCount, #messages)])
    
    if success then
        Log.debug("Updated message label text successfully")
    else
        Log.warn("Failed to update message label text")
    end
    
    -- 返回true表示事件已处理，防止程序退出
    return true
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
