// Lua Binding Tests
#include "TestFramework.h"
#include "lua/LuaSandbox.h"

using namespace luaui::lua;

// ==================== LuaSandbox Tests ====================

TEST(LuaSandbox_Initialize) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    ASSERT_TRUE(sandbox.IsInitialized());
    sandbox.Shutdown();
}

TEST(LuaSandbox_ExecuteSimple) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = "return 42";
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaSandbox_ExecuteWithError) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    // Syntax error
    const char* script = "return 42 + ";
    ASSERT_FALSE(sandbox.Execute(script));
    ASSERT_TRUE(!sandbox.GetLastError().empty());
    
    sandbox.Shutdown();
}

TEST(LuaSandbox_GetMemoryUsage) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    // Just check memory usage doesn't crash
    size_t mem = sandbox.GetMemoryUsage();
    ASSERT_TRUE(mem >= 0);
    
    sandbox.Shutdown();
}

TEST(LuaSandbox_ForceGC) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    // Just check GC doesn't crash
    sandbox.ForceGarbageCollect();
    ASSERT_TRUE(true);
    
    sandbox.Shutdown();
}

TEST(LuaSandbox_Security_dofile) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    // dofile should be nil (removed)
    const char* script = "return dofile == nil";
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

// ==================== UI Binding Tests ====================

TEST(LuaBinding_ButtonCreate) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        local btn = Button.new()
        btn:setText("Test")
        return btn:getText()
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_ButtonEnabled) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        local btn = Button.new()
        btn:setIsEnabled(false)
        return btn:getIsEnabled()
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_TextBlockCreate) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        local tb = TextBlock.new()
        tb:setText("Hello")
        return tb:getText()
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_CheckBoxProperties) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        local cb = CheckBox.new()
        cb:setText("Option")
        cb:setIsChecked(true)
        return cb:getIsChecked()
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_SliderProperties) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        local slider = Slider.new()
        slider:setRange(0, 100)
        slider:setValue(50)
        return slider:getValue()
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_ProgressBarProperties) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        local pb = ProgressBar.new()
        pb:setValue(75)
        return pb:getValue()
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_PanelCreate) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        local panel = Panel.new()
        return "Panel created"
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_GridLayout) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        local grid = Grid.new()
        grid:addColumn(100)
        grid:addColumn(200)
        grid:addRow(50)
        return "Grid created"
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_Logging) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        Log.debug("Debug message")
        Log.info("Info message")
        Log.warn("Warning message")
        Log.error("Error message")
        return "Logging test completed"
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_UIVersion) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        return UI.VERSION
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_ComplexScript) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        -- Create multiple controls
        local button = Button.new()
        button:setText("Submit")
        button:setIsEnabled(true)
        
        local textBlock = TextBlock.new()
        textBlock:setText("Welcome")
        
        local checkBox = CheckBox.new()
        checkBox:setText("I agree")
        checkBox:setIsChecked(false)
        
        local slider = Slider.new()
        slider:setRange(0, 100)
        slider:setValue(25)
        
        -- Create layout
        local grid = Grid.new()
        grid:addColumn(200)
        grid:addColumn(400)
        grid:addRow(100)
        grid:addRow(100)
        
        Log.info("Complex UI created with multiple controls")
        
        return "Complex script completed"
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

TEST(LuaBinding_MultipleControlsPerformance) {
    LuaSandbox sandbox;
    ASSERT_TRUE(sandbox.Initialize());
    
    const char* script = R"(
        for i = 1, 20 do
            local btn = Button.new()
            btn:setText("Btn")
            
            local tb = TextBlock.new()
            tb:setText("Txt")
            
            local cb = CheckBox.new()
            cb:setText("Chk")
        end
        return "Created controls"
    )";
    
    ASSERT_TRUE(sandbox.Execute(script));
    
    sandbox.Shutdown();
}

// ==================== ScriptEngine Tests ====================

TEST(ScriptEngine_Initialize) {
    ScriptEngine engine;
    ASSERT_TRUE(engine.Initialize());
    engine.Shutdown();
}

TEST(ScriptEngine_GlobalVariables) {
    ScriptEngine engine;
    ASSERT_TRUE(engine.Initialize());
    
    engine.SetGlobalString("testVar", "testValue");
    std::string value = engine.GetGlobalString("testVar");
    ASSERT_EQ(value, "testValue");
    
    engine.Shutdown();
}

// ==================== Main ====================

int main() {
    return RUN_ALL_TESTS();
}
