// Lua Binding Implementation - Phase 10
#include "LuaSandbox.h"
#include "Window.h"
#include "Controls.h"
#include "Button.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "CheckBox.h"
#include "Slider.h"
#include "ProgressBar.h"
#include "Panel.h"
#include "layouts/Grid.h"
#include "Logger.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <string>
#include <memory>
#include <iostream>
#include <windows.h>

namespace luaui {
namespace lua {

// ==================== Helper Functions ====================

static std::string WStringToString(const std::wstring& wstr) {
    std::string result;
    for (wchar_t wc : wstr) {
        if (wc < 128) {
            result += static_cast<char>(wc);
        } else {
            result += '?';
        }
    }
    return result;
}

static std::wstring StringToWString(const std::string& str) {
    std::wstring result;
    for (char c : str) {
        result += static_cast<wchar_t>(static_cast<unsigned char>(c));
    }
    return result;
}

// ==================== LuaBinding ====================

void LuaBinding::RegisterAll(lua_State* L) {
    if (!L) return;
    
    RegisterUIElements(L);
    RegisterLogger(L);
    RegisterUIGlobal(L);
}

void LuaBinding::RegisterUIElements(lua_State* L) {
    RegisterButton(L);
    RegisterTextBlock(L);
    RegisterCheckBox(L);
    RegisterSlider(L);
    RegisterProgressBar(L);
    RegisterPanel(L);
    RegisterStackPanel(L);
    RegisterGrid(L);
}

// ==================== Button ====================

void LuaBinding::RegisterButton(lua_State* L) {
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto button = std::make_shared<luaui::controls::Button>();
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_newuserdata(L, sizeof(std::shared_ptr<luaui::controls::Button>))
        );
        new(ptr) std::shared_ptr<luaui::controls::Button>(button);
        luaL_getmetatable(L, "LuaUI.Button");
        lua_setmetatable(L, -2);
        return 1;
    });
    lua_setfield(L, -2, "new");
    
    lua_setglobal(L, "Button");
    
    luaL_newmetatable(L, "LuaUI.Button");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        ptr->~shared_ptr();
        return 0;
    });
    lua_setfield(L, -2, "__gc");
    
    lua_newtable(L);
    
    // setText
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        const char* text = luaL_checkstring(L, 2);
        (*ptr)->SetText(StringToWString(text));
        return 0;
    });
    lua_setfield(L, -2, "setText");
    
    // getText
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        lua_pushstring(L, WStringToString((*ptr)->GetText()).c_str());
        return 1;
    });
    lua_setfield(L, -2, "getText");
    
    // setIsEnabled
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        bool enabled = lua_toboolean(L, 2);
        (*ptr)->SetIsEnabled(enabled);
        return 0;
    });
    lua_setfield(L, -2, "setIsEnabled");
    
    // getIsEnabled
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        lua_pushboolean(L, (*ptr)->GetIsEnabled());
        return 1;
    });
    lua_setfield(L, -2, "getIsEnabled");

    // setForeground (hex color string like "#FF0000")
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        const char* hex = luaL_checkstring(L, 2);
        uint32_t colorVal = static_cast<uint32_t>(std::stoul(hex, nullptr, 16));
        (*ptr)->SetForeground(luaui::rendering::Color::FromHex(colorVal));
        return 0;
    });
    lua_setfield(L, -2, "setForeground");

    // setCornerRadius
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        float radius = static_cast<float>(luaL_checknumber(L, 2));
        (*ptr)->SetCornerRadius(luaui::rendering::CornerRadius(radius));
        return 0;
    });
    lua_setfield(L, -2, "setCornerRadius");

    // setBorderBrush (hex color string)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        const char* hex = luaL_checkstring(L, 2);
        uint32_t colorVal = static_cast<uint32_t>(std::stoul(hex, nullptr, 16));
        (*ptr)->SetBorderBrush(luaui::rendering::Color::FromHex(colorVal));
        return 0;
    });
    lua_setfield(L, -2, "setBorderBrush");

    // setBorderThickness
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        float thickness = static_cast<float>(luaL_checknumber(L, 2));
        (*ptr)->SetBorderThickness(thickness);
        return 0;
    });
    lua_setfield(L, -2, "setBorderThickness");

    // setFontSize
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Button>*>(
            lua_touserdata(L, 1)
        );
        float size = static_cast<float>(luaL_checknumber(L, 2));
        (*ptr)->SetFontSize(size);
        return 0;
    });
    lua_setfield(L, -2, "setFontSize");
    
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

// ==================== TextBlock ====================

void LuaBinding::RegisterTextBlock(lua_State* L) {
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto textBlock = std::make_shared<luaui::controls::TextBlock>();
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::TextBlock>*>(
            lua_newuserdata(L, sizeof(std::shared_ptr<luaui::controls::TextBlock>))
        );
        new(ptr) std::shared_ptr<luaui::controls::TextBlock>(textBlock);
        luaL_getmetatable(L, "LuaUI.TextBlock");
        lua_setmetatable(L, -2);
        return 1;
    });
    lua_setfield(L, -2, "new");
    
    lua_setglobal(L, "TextBlock");
    
    luaL_newmetatable(L, "LuaUI.TextBlock");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::TextBlock>*>(
            lua_touserdata(L, 1)
        );
        ptr->~shared_ptr();
        return 0;
    });
    lua_setfield(L, -2, "__gc");
    
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::TextBlock>*>(
            lua_touserdata(L, 1)
        );
        const char* text = luaL_checkstring(L, 2);
        (*ptr)->SetText(StringToWString(text));
        return 0;
    });
    lua_setfield(L, -2, "setText");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::TextBlock>*>(
            lua_touserdata(L, 1)
        );
        lua_pushstring(L, WStringToString((*ptr)->GetText()).c_str());
        return 1;
    });
    lua_setfield(L, -2, "getText");
    
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

// ==================== CheckBox ====================

void LuaBinding::RegisterCheckBox(lua_State* L) {
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto checkBox = std::make_shared<luaui::controls::CheckBox>();
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::CheckBox>*>(
            lua_newuserdata(L, sizeof(std::shared_ptr<luaui::controls::CheckBox>))
        );
        new(ptr) std::shared_ptr<luaui::controls::CheckBox>(checkBox);
        luaL_getmetatable(L, "LuaUI.CheckBox");
        lua_setmetatable(L, -2);
        return 1;
    });
    lua_setfield(L, -2, "new");
    
    lua_setglobal(L, "CheckBox");
    
    luaL_newmetatable(L, "LuaUI.CheckBox");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::CheckBox>*>(
            lua_touserdata(L, 1)
        );
        ptr->~shared_ptr();
        return 0;
    });
    lua_setfield(L, -2, "__gc");
    
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::CheckBox>*>(
            lua_touserdata(L, 1)
        );
        const char* text = luaL_checkstring(L, 2);
        (*ptr)->SetText(StringToWString(text));
        return 0;
    });
    lua_setfield(L, -2, "setText");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::CheckBox>*>(
            lua_touserdata(L, 1)
        );
        bool checked = lua_toboolean(L, 2);
        (*ptr)->SetIsChecked(checked);
        return 0;
    });
    lua_setfield(L, -2, "setIsChecked");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::CheckBox>*>(
            lua_touserdata(L, 1)
        );
        lua_pushboolean(L, (*ptr)->GetIsChecked());
        return 1;
    });
    lua_setfield(L, -2, "getIsChecked");
    
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

// ==================== Slider ====================

void LuaBinding::RegisterSlider(lua_State* L) {
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto slider = std::make_shared<luaui::controls::Slider>();
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Slider>*>(
            lua_newuserdata(L, sizeof(std::shared_ptr<luaui::controls::Slider>))
        );
        new(ptr) std::shared_ptr<luaui::controls::Slider>(slider);
        luaL_getmetatable(L, "LuaUI.Slider");
        lua_setmetatable(L, -2);
        return 1;
    });
    lua_setfield(L, -2, "new");
    
    lua_setglobal(L, "Slider");
    
    luaL_newmetatable(L, "LuaUI.Slider");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Slider>*>(
            lua_touserdata(L, 1)
        );
        ptr->~shared_ptr();
        return 0;
    });
    lua_setfield(L, -2, "__gc");
    
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Slider>*>(
            lua_touserdata(L, 1)
        );
        double min = luaL_checknumber(L, 2);
        double max = luaL_checknumber(L, 3);
        (*ptr)->SetMinimum(min);
        (*ptr)->SetMaximum(max);
        return 0;
    });
    lua_setfield(L, -2, "setRange");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Slider>*>(
            lua_touserdata(L, 1)
        );
        double value = luaL_checknumber(L, 2);
        (*ptr)->SetValue(value);
        return 0;
    });
    lua_setfield(L, -2, "setValue");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Slider>*>(
            lua_touserdata(L, 1)
        );
        lua_pushnumber(L, (*ptr)->GetValue());
        return 1;
    });
    lua_setfield(L, -2, "getValue");
    
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

// ==================== ProgressBar ====================

void LuaBinding::RegisterProgressBar(lua_State* L) {
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto progress = std::make_shared<luaui::controls::ProgressBar>();
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::ProgressBar>*>(
            lua_newuserdata(L, sizeof(std::shared_ptr<luaui::controls::ProgressBar>))
        );
        new(ptr) std::shared_ptr<luaui::controls::ProgressBar>(progress);
        luaL_getmetatable(L, "LuaUI.ProgressBar");
        lua_setmetatable(L, -2);
        return 1;
    });
    lua_setfield(L, -2, "new");
    
    lua_setglobal(L, "ProgressBar");
    
    luaL_newmetatable(L, "LuaUI.ProgressBar");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::ProgressBar>*>(
            lua_touserdata(L, 1)
        );
        ptr->~shared_ptr();
        return 0;
    });
    lua_setfield(L, -2, "__gc");
    
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::ProgressBar>*>(
            lua_touserdata(L, 1)
        );
        double value = luaL_checknumber(L, 2);
        (*ptr)->SetValue(value);
        return 0;
    });
    lua_setfield(L, -2, "setValue");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::ProgressBar>*>(
            lua_touserdata(L, 1)
        );
        lua_pushnumber(L, (*ptr)->GetValue());
        return 1;
    });
    lua_setfield(L, -2, "getValue");
    
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

// ==================== Panel ====================

void LuaBinding::RegisterPanel(lua_State* L) {
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto panel = std::make_shared<luaui::controls::Panel>();
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Panel>*>(
            lua_newuserdata(L, sizeof(std::shared_ptr<luaui::controls::Panel>))
        );
        new(ptr) std::shared_ptr<luaui::controls::Panel>(panel);
        luaL_getmetatable(L, "LuaUI.Panel");
        lua_setmetatable(L, -2);
        return 1;
    });
    lua_setfield(L, -2, "new");
    
    lua_setglobal(L, "Panel");
    
    luaL_newmetatable(L, "LuaUI.Panel");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Panel>*>(
            lua_touserdata(L, 1)
        );
        ptr->~shared_ptr();
        return 0;
    });
    lua_setfield(L, -2, "__gc");
    
    lua_newtable(L);
    
    // addChild - accepts any control userdata with Control metatable
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* panelPtr = static_cast<std::shared_ptr<luaui::controls::Panel>*>(
            lua_touserdata(L, 1)
        );
        // The second argument should be a userdata with a shared_ptr to a Control
        // We need to cast it - this is a simplification
        if (lua_isuserdata(L, 2)) {
            // Try to get as Control base pointer
            // This is a workaround - in production we'd need proper inheritance handling
            auto* childPtr = static_cast<std::shared_ptr<luaui::Control>*>(
                lua_touserdata(L, 2)
            );
            if (childPtr && *childPtr) {
                (*panelPtr)->AddChild(*childPtr);
            }
        }
        return 0;
    });
    lua_setfield(L, -2, "addChild");
    
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

// ==================== StackPanel ====================

void LuaBinding::RegisterStackPanel(lua_State* L) {
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto panel = std::make_shared<luaui::controls::StackPanel>();
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::StackPanel>*>(
            lua_newuserdata(L, sizeof(std::shared_ptr<luaui::controls::StackPanel>))
        );
        new(ptr) std::shared_ptr<luaui::controls::StackPanel>(panel);
        luaL_getmetatable(L, "LuaUI.StackPanel");
        lua_setmetatable(L, -2);
        return 1;
    });
    lua_setfield(L, -2, "new");
    
    lua_setglobal(L, "StackPanel");
    
    luaL_newmetatable(L, "LuaUI.StackPanel");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::StackPanel>*>(
            lua_touserdata(L, 1)
        );
        ptr->~shared_ptr();
        return 0;
    });
    lua_setfield(L, -2, "__gc");
    
    lua_newtable(L);
    
    // setOrientation - 0 = Vertical, 1 = Horizontal
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::StackPanel>*>(
            lua_touserdata(L, 1)
        );
        int orientation = static_cast<int>(luaL_checkinteger(L, 2));
        if (orientation == 0) {
            (*ptr)->SetOrientation(luaui::controls::StackPanel::Orientation::Vertical);
        } else {
            (*ptr)->SetOrientation(luaui::controls::StackPanel::Orientation::Horizontal);
        }
        return 0;
    });
    lua_setfield(L, -2, "setOrientation");
    
    // setSpacing
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::StackPanel>*>(
            lua_touserdata(L, 1)
        );
        float spacing = static_cast<float>(luaL_checknumber(L, 2));
        (*ptr)->SetSpacing(spacing);
        return 0;
    });
    lua_setfield(L, -2, "setSpacing");
    
    // addChild (inherited from Panel)
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* panelPtr = static_cast<std::shared_ptr<luaui::controls::StackPanel>*>(
            lua_touserdata(L, 1)
        );
        if (lua_isuserdata(L, 2)) {
            auto* childPtr = static_cast<std::shared_ptr<luaui::Control>*>(
                lua_touserdata(L, 2)
            );
            if (childPtr && *childPtr) {
                (*panelPtr)->AddChild(*childPtr);
            }
        }
        return 0;
    });
    lua_setfield(L, -2, "addChild");
    
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

// ==================== Grid ====================

void LuaBinding::RegisterGrid(lua_State* L) {
    lua_newtable(L);
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto grid = std::make_shared<luaui::controls::Grid>();
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Grid>*>(
            lua_newuserdata(L, sizeof(std::shared_ptr<luaui::controls::Grid>))
        );
        new(ptr) std::shared_ptr<luaui::controls::Grid>(grid);
        luaL_getmetatable(L, "LuaUI.Grid");
        lua_setmetatable(L, -2);
        return 1;
    });
    lua_setfield(L, -2, "new");
    
    lua_setglobal(L, "Grid");
    
    luaL_newmetatable(L, "LuaUI.Grid");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Grid>*>(
            lua_touserdata(L, 1)
        );
        ptr->~shared_ptr();
        return 0;
    });
    lua_setfield(L, -2, "__gc");
    
    lua_newtable(L);
    
    // addColumn
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Grid>*>(
            lua_touserdata(L, 1)
        );
        float width = static_cast<float>(luaL_checknumber(L, 2));
        (*ptr)->AddColumn(luaui::controls::GridLength::Pixel(width));
        return 0;
    });
    lua_setfield(L, -2, "addColumn");
    
    // addRow
    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto* ptr = static_cast<std::shared_ptr<luaui::controls::Grid>*>(
            lua_touserdata(L, 1)
        );
        float height = static_cast<float>(luaL_checknumber(L, 2));
        (*ptr)->AddRow(luaui::controls::GridLength::Pixel(height));
        return 0;
    });
    lua_setfield(L, -2, "addRow");
    
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

// Logger binding is implemented in LuaBinding_Logger.cpp

// ==================== UIGlobal ====================

void LuaBinding::RegisterUIGlobal(lua_State* L) {
    lua_newtable(L);
    
    // VERSION
    lua_pushstring(L, "1.0.0");
    lua_setfield(L, -2, "VERSION");
    
    lua_setglobal(L, "UI");
}

// Stub implementations for other register functions
void LuaBinding::RegisterWindow(lua_State* L) { (void)L; }
void LuaBinding::RegisterTextBox(lua_State* L) { (void)L; }
void LuaBinding::RegisterImage(lua_State* L) { (void)L; }
void LuaBinding::RegisterListBox(lua_State* L) { (void)L; }
void LuaBinding::RegisterComboBox(lua_State* L) { (void)L; }
void LuaBinding::RegisterTabControl(lua_State* L) { (void)L; }
void LuaBinding::RegisterTreeView(lua_State* L) { (void)L; }
void LuaBinding::RegisterDataGrid(lua_State* L) { (void)L; }
void LuaBinding::RegisterMenu(lua_State* L) { (void)L; }
void LuaBinding::RegisterToolBar(lua_State* L) { (void)L; }
void LuaBinding::RegisterStatusBar(lua_State* L) { (void)L; }
void LuaBinding::RegisterDialog(lua_State* L) { (void)L; }
void LuaBinding::RegisterShapes(lua_State* L) { (void)L; }
void LuaBinding::RegisterProperties(lua_State* L) { (void)L; }
// Note: RegisterEvents, RegisterCommands, RegisterLogger are implemented in separate files
void LuaBinding::RegisterBindings(lua_State* L) { (void)L; }
void LuaBinding::RegisterAnimations(lua_State* L) { (void)L; }
void LuaBinding::RegisterResources(lua_State* L) { (void)L; }
void LuaBinding::RegisterDialogs(lua_State* L) { (void)L; }
void LuaBinding::RegisterStorage(lua_State* L) { (void)L; }
void LuaBinding::RegisterTask(lua_State* L) { (void)L; }

// ==================== Window Exposure ====================
// Exposes a Window instance to Lua for setting root control

static luaui::Window* s_exposedWindow = nullptr;

void LuaBinding::ExposeWindow(lua_State* L, void* window) {
    if (!L || !window) return;
    s_exposedWindow = static_cast<luaui::Window*>(window);
    
    // Create or get Host table
    lua_getglobal(L, "Host");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    
    // Host.SetRoot(control) - Set window root control
    lua_pushcfunction(L, [](lua_State* L) -> int {
        // Debug output to file
        FILE* fp = fopen("setroot_debug.log", "a");
        if (fp) {
            fprintf(fp, "[C++ Host.SetRoot] Called\n");
            fflush(fp);
        }
        
        if (!s_exposedWindow) {
            if (fp) { fprintf(fp, "[C++ Host.SetRoot] s_exposedWindow is null\n"); fclose(fp); }
            return 0;
        }
        if (!lua_isuserdata(L, 1)) {
            if (fp) { fprintf(fp, "[C++ Host.SetRoot] not userdata\n"); fclose(fp); }
            return 0;
        }
        
        void* userdata = lua_touserdata(L, 1);
        if (fp) { fprintf(fp, "[C++ Host.SetRoot] userdata=%p\n", userdata); fflush(fp); }
        
        // Try StackPanel first
        std::shared_ptr<luaui::controls::StackPanel>* panelPtr = 
            static_cast<std::shared_ptr<luaui::controls::StackPanel>*>(userdata);
        if (fp) { fprintf(fp, "[C++ Host.SetRoot] panelPtr=%p\n", (void*)panelPtr); fflush(fp); }
        
        if (panelPtr) {
            if (fp) { fprintf(fp, "[C++ Host.SetRoot] panelPtr is not null\n"); fflush(fp); }
            if (*panelPtr) {
                if (fp) { fprintf(fp, "[C++ Host.SetRoot] *panelPtr is valid, setting root\n"); fflush(fp); }
                std::shared_ptr<luaui::Control> control = *panelPtr;
                s_exposedWindow->SetRoot(control);
                if (fp) { fprintf(fp, "[C++ Host.SetRoot] SetRoot called successfully\n"); fclose(fp); }
                return 0;
            } else {
                if (fp) { fprintf(fp, "[C++ Host.SetRoot] *panelPtr is empty\n"); fflush(fp); }
            }
        } else {
            if (fp) { fprintf(fp, "[C++ Host.SetRoot] panelPtr is null\n"); fflush(fp); }
        }
        
        if (fp) fclose(fp);
        return 0;
    });
    lua_setfield(L, -2, "SetRoot");
    
    lua_setglobal(L, "Host");
}

} // namespace lua
} // namespace luaui
