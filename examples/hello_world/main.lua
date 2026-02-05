-- Hello World Lua Script for LuaUI

-- 设置日志级别
Log.setLevel("DEBUG")

-- 全局变量
local clickCount = 0

-- 按钮点击事件处理
function onHelloClick()
    clickCount = clickCount + 1

    Log.info("Button clicked, count: " .. clickCount)

    local messageLabel = UI:getControl("lblMessage")
    local messages = {
        "Hello, LuaUI!",
        "Great job!",
        "Keep clicking!",
        "You clicked " .. clickCount .. " times!",
        "Lua is awesome!"
    }

    messageLabel:setProperty("text", messages[math.min(clickCount, #messages)])

    Log.debug("Updated message label text")
end

-- 退出按钮点击事件
function onExitClick()
    Log.warn("User clicked exit button")
    UI:exit()
end

-- 窗口初始化
function onInit()
    Log.info("Hello World application initialized")
    Log.debug("Lua environment ready")
end

-- 窗口关闭
function onClose()
    Log.info("Hello World application closing")
end
