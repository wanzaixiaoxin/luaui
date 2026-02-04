-- Hello World Lua Script for LuaUI

-- 全局变量
local clickCount = 0

-- 按钮点击事件处理
function onHelloClick()
    clickCount = clickCount + 1
    
    local messageLabel = UI:getControl("lblMessage")
    local messages = {
        "Hello, LuaUI!",
        "Great job!",
        "Keep clicking!",
        "You clicked " .. clickCount .. " times!",
        "Lua is awesome!"
    }
    
    messageLabel:setProperty("text", messages[math.min(clickCount, #messages)])
end

-- 退出按钮点击事件
function onExitClick()
    UI:exit()
end

-- 窗口初始化
function onInit()
    print("LuaUI Hello World application initialized")
end

-- 窗口关闭
function onClose()
    print("LuaUI Hello World application closing")
end
