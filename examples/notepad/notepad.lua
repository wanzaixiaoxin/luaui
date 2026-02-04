-- LuaUI Notepad Example
-- 完整的记事本应用

-- 全局变量
local currentFile = ""
local isModified = false
local wordWrap = true

-- ==================== 文件操作 ====================

function menuNew_onClick()
    if isModified then
        local result = askSaveChanges()
        if result == "cancel" then
            return
        elseif result == "yes" then
            menuSave_onClick()
        end
    end
    
    currentFile = ""
    isModified = false
    UI:getProperty("editText", "text", "")
    updateWindowTitle()
    updateStatusBar()
end

function menuOpen_onClick()
    if isModified then
        local result = askSaveChanges()
        if result == "cancel" then
            return
        elseif result == "yes" then
            menuSave_onClick()
        end
    end
    
    -- TODO: 实现文件打开对话框
    print("Open file dialog...")
end

function menuSave_onClick()
    if currentFile == "" then
        menuSaveAs_onClick()
        return
    end
    
    -- TODO: 实现文件保存
    isModified = false
    updateWindowTitle()
    updateStatusBar()
end

function menuSaveAs_onClick()
    -- TODO: 实现另存为对话框
    print("Save as dialog...")
end

function menuExit_onClick()
    if isModified then
        local result = askSaveChanges()
        if result == "cancel" then
            return
        elseif result == "yes" then
            menuSave_onClick()
        end
    end
    
    UI:exit()
end

-- ==================== 编辑操作 ====================

function menuUndo_onClick()
    -- TODO: 实现撤销
    print("Undo")
end

function menuRedo_onClick()
    -- TODO: 实现重做
    print("Redo")
end

function menuCut_onClick()
    -- TODO: 实现剪切
    print("Cut")
end

function menuCopy_onClick()
    -- TODO: 实现复制
    print("Copy")
end

function menuPaste_onClick()
    -- TODO: 实现粘贴
    print("Paste")
end

function menuSelectAll_onClick()
    local editor = UI:getControl("editText")
    editor:setProperty("selstart", "0")
    editor:setProperty("selend", "1000000")
end

-- ==================== 格式操作 ====================

function menuFont_onClick()
    -- TODO: 实现字体选择对话框
    print("Font dialog...")
end

function menuWordWrap_onClick()
    wordWrap = not wordWrap
    local editor = UI:getControl("editText")
    editor:setProperty("multiline", wordWrap and "true" or "false")
    
    -- 更新菜单项状态
    -- TODO: 设置菜单项选中状态
end

-- ==================== 帮助 ====================

function menuAbout_onClick()
    print("LuaUI Notepad v1.0")
    print("A simple notepad application using LuaUI framework")
    print("Based on MFC, XML layout, and Lua scripting")
end

-- ==================== 工具栏事件 ====================

function btnNew_onClick()
    menuNew_onClick()
end

function btnOpen_onClick()
    menuOpen_onClick()
end

function btnSave_onClick()
    menuSave_onClick()
end

function btnCut_onClick()
    menuCut_onClick()
end

function btnCopy_onClick()
    menuCopy_onClick()
end

function btnPaste_onClick()
    menuPaste_onClick()
end

-- ==================== 编辑器事件 ====================

function editText_onChanged()
    isModified = true
    updateWindowTitle()
    updateStatusBar()
end

function editText_onKeyPress(key)
    -- 记录光标位置
    updateStatusBar()
end

-- ==================== 辅助函数 ====================

function askSaveChanges()
    local result = ""
    -- TODO: 显示确认对话框
    -- result = showDialog("Save changes?", "Question", ["Yes", "No", "Cancel"])
    return result
end

function updateWindowTitle()
    local title = "Notepad - LuaUI"
    if currentFile ~= "" then
        title = currentFile .. " - Notepad - LuaUI"
    end
    
    if isModified then
        title = title .. " *"
    end
    
    local mainWindow = UI:getControl("mainWindow")
    mainWindow:setProperty("title", title)
end

function updateStatusBar()
    local statusBar = UI:getControl("mainStatusBar")
    local editor = UI:getControl("editText")
    
    local status = "Ready"
    if isModified then
        status = "Modified"
    end
    
    -- TODO: 获取光标位置
    local line = 1
    local col = 1
    
    -- TODO: 获取文本长度
    local length = 0
    
    -- 更新状态栏
    -- TODO: 实现状态栏面板更新
end

-- ==================== 初始化 ====================

function onInit()
    print("LuaUI Notepad initialized")
    
    -- 绑定所有菜单项事件
    UI:bindEvent("menuNew", "onClick", menuNew_onClick)
    UI:bindEvent("menuOpen", "onClick", menuOpen_onClick)
    UI:bindEvent("menuSave", "onClick", menuSave_onClick)
    UI:bindEvent("menuSaveAs", "onClick", menuSaveAs_onClick)
    UI:bindEvent("menuExit", "onClick", menuExit_onClick)
    
    UI:bindEvent("menuUndo", "onClick", menuUndo_onClick)
    UI:bindEvent("menuRedo", "onClick", menuRedo_onClick)
    UI:bindEvent("menuCut", "onClick", menuCut_onClick)
    UI:bindEvent("menuCopy", "onClick", menuCopy_onClick)
    UI:bindEvent("menuPaste", "onClick", menuPaste_onClick)
    UI:bindEvent("menuSelectAll", "onClick", menuSelectAll_onClick)
    
    UI:bindEvent("menuFont", "onClick", menuFont_onClick)
    UI:bindEvent("menuWordWrap", "onClick", menuWordWrap_onClick)
    
    UI:bindEvent("menuAbout", "onClick", menuAbout_onClick)
    
    -- 绑定工具栏按钮事件
    UI:bindEvent("btnNew", "onClick", btnNew_onClick)
    UI:bindEvent("btnOpen", "onClick", btnOpen_onClick)
    UI:bindEvent("btnSave", "onClick", btnSave_onClick)
    UI:bindEvent("btnCut", "onClick", btnCut_onClick)
    UI:bindEvent("btnCopy", "onClick", btnCopy_onClick)
    UI:bindEvent("btnPaste", "onClick", btnPaste_onClick)
    
    -- 绑定编辑器事件
    UI:bindEvent("editText", "onChanged", editText_onChanged)
    UI:bindEvent("editText", "onKeyPress", editText_onKeyPress)
    
    -- 设置初始状态
    updateWindowTitle()
    updateStatusBar()
end

function onClose()
    print("LuaUI Notepad closing")
end
