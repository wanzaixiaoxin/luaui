# LuaUI Project - AI Coding Agent Guide

## Project Overview

**LuaUI** is a commercial-grade Windows UI framework with the following key characteristics:

- **Rendering**: Hardware-accelerated 2D graphics using Direct2D 1, DirectWrite, and WIC
- **Layout System**: Declarative XML layouts (XAML-like) with flexible layout containers
- **Data Binding**: Full MVVM (Model-View-ViewModel) support with OneWay, TwoWay, and OneTime bindings
- **Scripting**: Lua 5.4 integration with sandboxed execution environment
- **Language**: Modern C++17 standard
- **Platform**: Windows only (requires Windows SDK)
- **License**: MIT License

## Technology Stack

| Component | Technology |
|-----------|------------|
| Build System | CMake 3.15+ |
| Language | C++17 |
| Rendering | Direct2D 1, DirectWrite, WIC |
| XML Parsing | TinyXML2 |
| Scripting | Lua 5.4 (pre-built binaries in `libs/`) |
| Windows API | Win32, DXGI |

## Project Structure

```
luaui/
├── CMakeLists.txt              # Root CMake configuration
├── src/luaui/                  # Main source code
│   ├── core/                   # Core framework (Window, Control, Components, Interfaces)
│   │   ├── Components/         # Component-based architecture
│   │   │   ├── Component.h/cpp
│   │   │   ├── LayoutComponent.h/cpp
│   │   │   ├── RenderComponent.h/cpp
│   │   │   └── InputComponent.h/cpp
│   │   ├── Interfaces/         # Core interfaces
│   │   │   ├── IControl.h
│   │   │   ├── IRenderable.h
│   │   │   ├── ILayoutable.h
│   │   │   ├── IInputHandler.h
│   │   │   └── IStyleable.h
│   │   ├── Control.h/cpp
│   │   ├── Window.h/cpp
│   │   ├── Dispatcher.h/cpp
│   │   └── Delegate.h
│   ├── controls/               # UI controls (Button, TextBox, etc.)
│   │   ├── Button.h/cpp
│   │   ├── TextBlock.h/cpp
│   │   ├── TextBox.h/cpp
│   │   ├── CheckBox.h/cpp
│   │   ├── Slider.h/cpp
│   │   ├── ProgressBar.h/cpp
│   │   ├── Image.h/cpp
│   │   ├── Panel.h/cpp
│   │   ├── Shapes.h/cpp
│   │   ├── DataGrid.h/cpp
│   │   ├── TabControl.h/cpp
│   │   ├── layouts/            # Layout containers
│   │   │   ├── StackPanel.h/cpp
│   │   │   ├── Grid.h/cpp
│   │   │   ├── Canvas.h/cpp
│   │   │   ├── DockPanel.h/cpp
│   │   │   ├── WrapPanel.h/cpp
│   │   │   ├── ScrollViewer.h/cpp
│   │   │   └── Viewbox.h/cpp
│   │   └── Controls.h
│   ├── rendering/              # Rendering engine
│   │   ├── Types.h             # Color, Point, Size, Rect, Transform
│   │   ├── IRenderContext.h    # Main rendering interface
│   │   ├── IRenderEngine.h
│   │   ├── IBitmap.h
│   │   ├── IBrush.h
│   │   ├── IGeometry.h
│   │   ├── ResourceCache.h/cpp
│   │   └── d2d/                # Direct2D implementation
│   │       ├── D2DRenderContext.h/cpp
│   │       ├── D2DRenderEngine.h/cpp
│   │       ├── D2DBitmap.h/cpp
│   │       ├── D2DBrush.h/cpp
│   │       └── D2DTextFormat.h/cpp
│   ├── xml/                    # XML layout parsing
│   │   ├── XmlLoader.cpp
│   │   └── TypeConverter.cpp
│   ├── mvvm/                   # MVVM data binding system
│   │   ├── ViewModelBase.h/cpp
│   │   ├── BindingEngine.h/cpp
│   │   ├── MvvmXmlLoader.h/cpp
│   │   ├── Converters.h
│   │   └── XmlBindingExtension.h/cpp
│   ├── style/                  # Style system
│   │   ├── Theme.h/cpp
│   │   ├── Style.h/cpp
│   │   ├── Setter.h/cpp
│   │   ├── Trigger.h/cpp
│   │   └── ResourceDictionary.h/cpp
│   ├── lua/                    # Lua scripting bindings
│   │   ├── LuaSandbox.h/cpp
│   │   ├── LuaMvvmHost.h/cpp
│   │   ├── LuaAwareMvvmLoader.h/cpp
│   │   ├── LuaBinding.cpp
│   │   ├── LuaBinding_Logger.cpp
│   │   ├── LuaBinding_Events.cpp
│   │   └── LuaBinding_MVVM.h/cpp
│   └── utils/                  # Utilities
│       └── Logger.h/cpp
├── third_party/                # External dependencies
│   ├── lua/                    # Lua 5.4 headers
│   └── tinyxml2/               # XML parsing library
├── libs/                       # Pre-built Lua binaries
│   ├── lua.exe
│   ├── lua54.dll
│   └── lua54.lib
├── examples/                   # Example applications
│   ├── 01_controls_demo/       # Control showcase demo
│   ├── 02_mvvm_demo/           # MVVM data binding demo
│   └── 03_lua_mvvm_demo/       # Lua + MVVM integration demo
├── tests/                      # Unit tests with custom test framework
│   ├── TestFramework.h
│   └── test_*.cpp
├── docs/                       # Documentation (mostly in Chinese)
│   ├── architecture/           # Architecture design documents
│   └── api/                    # API documentation
└── resources/                  # Framework resources
```

## Build Instructions

### Prerequisites

- Windows OS
- Visual Studio 2019/2022 with C++ workload
- CMake 3.15 or higher

### Build Commands

```bash
# Create build directory
mkdir build && cd build

# Configure (default: examples ON, tests OFF)
cmake ..

# Or configure with specific options
cmake .. -DLUAUI_BUILD_TESTS=ON -DLUAUI_BUILD_EXAMPLES=ON

# Build
cmake --build . --config Release

# Run tests (if built with -DLUAUI_BUILD_TESTS=ON)
ctest -C Release
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `LUAUI_BUILD_EXAMPLES` | ON | Build example applications |
| `LUAUI_BUILD_TESTS` | OFF | Build test suite |

### Output Directories

- Executables: `${CMAKE_BINARY_DIR}/bin/`
- Libraries: `${CMAKE_BINARY_DIR}/lib/`

## Module Architecture

The project uses a modular CMake structure with interface libraries:

```
LuaUI (Interface Library)
├── LuaUI_Core           # Window, Control, Components, Interfaces
├── LuaUI_Controls       # Button, TextBox, Panel, etc.
├── LuaUI_ControlsLayout # Layout containers (StackPanel, Grid, Canvas, etc.)
├── LuaUI_Rendering      # IRenderContext + Types + Direct2D implementation
├── LuaUI_Style          # Theme, Style, ResourceDictionary
├── LuaUI_Xml            # XML layout loader
├── LuaUI_MVVM           # Data binding engine
└── LuaUI_Lua            # Lua scripting bindings
```

Each module is a static library with its own `CMakeLists.txt`.

## Code Style Guidelines

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes | PascalCase | `Control`, `StackPanel` |
| Functions | PascalCase | `GetWidth()`, `SetIsVisible()` |
| Variables | camelCase | `availableSize`, `isVisible` |
| Member Variables | m_ prefix + camelCase | `m_width`, `m_parent` |
| Constants | UPPER_CASE | `MAX_WIDTH` |
| Namespaces | lowercase | `luaui::controls` |
| Macros | UPPER_CASE with prefix | `LUAUI_API` |
| Files | PascalCase | `Control.h`, `StackPanel.cpp` |

### Code Format

- **Indentation**: 4 spaces (no tabs)
- **Comments**: Use Doxygen-style for public APIs
- **Include Guards**: Use `#pragma once`
- **Line Endings**: CRLF on Windows

### Example Header Pattern

```cpp
#pragma once

#include "Interfaces/IControl.h"
#include <string>

namespace luaui {

/**
 * @brief Brief description
 * @details Detailed description
 */
class Control : public interfaces::IControl {
public:
    Control();
    virtual ~Control();
    
    // Getters/Setters follow PascalCase
    bool GetIsVisible() const;
    void SetIsVisible(bool visible);
    
protected:
    virtual void OnRender(rendering::IRenderContext* context);
    
private:
    std::string m_name;  // Member with m_ prefix
    bool m_visible = true;
};

} // namespace luaui
```

## Testing Framework

The project uses a **custom header-only test framework** defined in `tests/TestFramework.h`.

### Writing Tests

```cpp
#include "TestFramework.h"
#include "Types.h"

using namespace luaui::rendering;

TEST(Color_Construction) {
    Color c(1.0f, 0.5f, 0.25f, 0.8f);
    ASSERT_EQ(c.r, 1.0f);
    ASSERT_NEAR(c.g, 0.5f, 0.01f);
}

TEST(Color_FromHex) {
    Color c = Color::FromHex(0xFF8040);
    ASSERT_NEAR(c.r, 1.0f, 0.01f);
}

int main() {
    return RUN_ALL_TESTS();
}
```

### Available Assertions

| Macro | Description |
|-------|-------------|
| `TEST(name)` | Define a test case |
| `ASSERT_TRUE(expr)` | Assert expression is true |
| `ASSERT_FALSE(expr)` | Assert expression is false |
| `ASSERT_EQ(expected, actual)` | Assert equality |
| `ASSERT_NE(a, b)` | Assert not equal |
| `ASSERT_FLOAT_EQ(e, a, epsilon)` | Assert float equality with tolerance |
| `ASSERT_NEAR(e, a, epsilon)` | Same as ASSERT_FLOAT_EQ |
| `ASSERT_NULL(ptr)` | Assert pointer is null |
| `ASSERT_NOT_NULL(ptr)` | Assert pointer is not null |
| `ASSERT_THROW(expr, type)` | Assert exception is thrown |
| `ASSERT_NO_THROW(expr)` | Assert no exception thrown |
| `EXPECT_TRUE/EXPECT_FALSE/EXPECT_EQ/EXPECT_NE` | Non-fatal variants |

### Running Tests

Tests are enabled with `-DLUAUI_BUILD_TESTS=ON`. Individual test executables:

| Test File | Target Name | Status |
|-----------|-------------|--------|
| `test_rendering.cpp` | RenderingTest | Enabled |
| `test_rendering_resources.cpp` | RenderingResourcesTest | Enabled |
| `test_core_control.cpp` | CoreControlTest | Enabled |
| `test_layout_grid.cpp` | GridLayoutTest | Enabled |
| `test_lua_binding.cpp` | LuaBindingTest | Enabled |
| `test_logger.cpp` | LoggerTest | Enabled |
| `test_layout.cpp` | LayoutTest | Disabled (API mismatch) |
| `test_core_delegates.cpp` | CoreDelegatesTest | Disabled (API mismatch) |
| `test_style_system.cpp` | StyleSystemTest | Disabled (API mismatch) |

## Key Design Patterns

### 1. Component-Based Architecture

Controls use a component system to separate concerns:

```cpp
class Control {
protected:
    virtual void InitializeComponents();  // Override to add components
    
private:
    components::ComponentHolder m_components;
    // LayoutComponent - handles layout (size, position, margin, padding)
    // RenderComponent - handles rendering (background, opacity, visibility)
    // InputComponent - handles input (mouse, keyboard, focus)
};
```

Access components via `GetLayout()`, `GetRender()`, `GetInput()` methods.

### 2. MVVM Data Binding

ViewModels inherit from `ViewModelBase` and implement `INotifyPropertyChanged`:

```cpp
class MyViewModel : public luaui::mvvm::ViewModelBase {
public:
    void SetValue(int value) {
        if (m_value != value) {
            m_value = value;
            RaisePropertyChanged("Value");  // Triggers UI update
        }
    }
};
```

XML binding syntax:
```xml
<TextBlock Text="{Binding UserName}" />
<TextBox Text="{Binding UserName, Mode=TwoWay}" />
```

### 3. Interface Segregation

Core interfaces defined in `src/luaui/core/Interfaces/`:
- `IControl` - Basic control operations (ID, Name, Visibility, Parent)
- `IRenderable` - Rendering capability
- `ILayoutable` - Layout capability (Measure, Arrange)
- `IInputHandler` - Input handling
- `IStyleable` - Styling support
- `INativeWindow` - Native window operations

### 4. Delegate Pattern

Event system uses template-based delegates:

```cpp
// In Control.h
Delegate<Control*> Click;
Delegate<Control*, double> ValueChanged;

// Usage
button->Click.Add([](Control* sender) {
    Logger::Info("Button clicked!");
});
```

## Lua Scripting

LuaUI provides comprehensive Lua bindings for UI automation and scripting.

### Lua Logging API

Lua scripts can use the `Log` table to write to the C++ logger:

```lua
-- Basic logging
Log.debug("Debug message")
Log.info("Info message")
Log.warn("Warning message")
Log.error("Error message")
Log.trace("Trace message")
Log.fatal("Fatal message")

-- Multiple arguments (space-separated)
Log.info("User", name, "logged in")

-- Formatted logging (printf-style)
Log.infof("User %s has %d messages", username, count)

-- Log level management
Log.setLevel("warn")  -- filter below warning
local level = Log.getLevel()

-- Global print() redirects to Log.info
print("This goes to the logger")
```

### Lua Event Binding

Lua functions can be registered as event handlers:

```lua
-- Button click event
local button = Button.new()
button:setText("Click Me")

local connection = button:onClick(function()
    Log.info("Button clicked!")
end)

-- Slider value changed
local slider = Slider.new()
slider:onValueChanged(function(value)
    Log.infof("Value: %s", value)
end)

-- Disconnect when done
connection:disconnect()
```

### Lua MVVM ViewModel

Lua ViewModels use `AutoViewModel` with automatic property notification:

```lua
-- Create ViewModel with auto-notification
local ViewModel = AutoViewModel.new():EnableAutoNotify()

-- Define properties (changes auto-trigger UI updates)
ViewModel.Title = "My Application"
ViewModel.Counter = 0

-- Define computed properties
ViewModel:DefineComputed("DisplayText", {"Counter"}, function(self)
    return "Count: " .. self.Counter
end)

-- Commands (functions ending with "Command" auto-bind to XML)
function ViewModel:IncrementCommand()
    self.Counter = self.Counter + 1
end

-- Register globally for framework to find
_G.ViewModelInstance = ViewModel
```

## Important Notes

### Windows-Only Platform

This framework is **Windows-only**. The CMakeLists.txt will fail on non-Windows:

```cmake
if(NOT WIN32)
    message(FATAL_ERROR "LuaUI currently only supports Windows")
endif()
```

### Thread Safety

- UI operations must be on the UI thread
- Use `Dispatcher` for cross-thread UI updates
- Controls have `VerifyUIThread()` for debugging

### Resource Management

- All controls use `std::shared_ptr` for ownership
- Render resources use `ResourceCache` for efficient reuse
- Direct2D objects are wrapped in COM smart pointers

### Third-Party Dependencies

| Library | Location | Usage |
|---------|----------|-------|
| Lua 5.4 | `third_party/lua/` | Header-only, for Lua scripting integration |
| TinyXML2 | `third_party/tinyxml2/` | XML parsing for layouts |
| Lua Binaries | `libs/` | Pre-built `lua54.dll` and `lua54.lib` |

### Rendering Types

Core rendering types in `src/luaui/rendering/Types.h`:

```cpp
// Color with premultiplied alpha support
Color c = Color::FromHex(0xFF8040);
Color c2 = Color::FromRGBA(255, 128, 64, 255);

// 2D geometry types
Point p(x, y);
Size s(width, height);
Rect r(x, y, width, height);
Thickness t(left, top, right, bottom);
CornerRadius cr(tl, tr, br, bl);
Transform tr = Transform::Translation(x, y);
```

## Security Considerations

Lua sandboxing is implemented with configurable security policies:

```cpp
LuaSandbox::SecurityPolicy policy;
policy.maxMemoryBytes = 64 * 1024 * 1024;  // 64MB
policy.maxExecutionTime = std::chrono::seconds(30);
policy.allowFileWrite = false;
policy.allowNetwork = false;
policy.allowLoadLibrary = false;
```

Default restrictions:
- Memory limit: 64MB
- Execution timeout: 30 seconds
- File write: Disabled
- Network access: Disabled
- Library loading: Disabled

## Examples Reference

| Example | Description |
|---------|-------------|
| 01_controls_demo | **Control System Demo** - Code-based control creation with TabControl showcase |
| 02_mvvm_demo | **MVVM Data Binding Demo** - XML-based with C++ ViewModel and declarative bindings |
| 03_lua_mvvm_demo | **Lua MVVM Demo** - XML (View) + Lua (ViewModel) architecture with auto-binding |

Run examples from build directory:
```bash
./bin/Release/01_controls_demo.exe
./bin/Release/02_mvvm_demo.exe
./bin/Release/03_lua_mvvm_demo.exe
```

## Documentation

Architecture documentation is primarily in Chinese and located in `docs/architecture/`:

| Document | Description |
|----------|-------------|
| `README.md` | Architecture overview and index |
| `DIRECTORY_STRUCTURE.md` | Project directory structure |
| `Commercial_UI_Framework_Design.md` | Overall architecture and design philosophy |
| `Render_Engine_Design.md` | Direct2D rendering engine design |
| `Layout_System.md` | Layout system design and usage |
| `MVVM_DataBinding_Design.md` | MVVM and data binding system |
| `Lua_Binding_Design.md` | Lua scripting API design |
| `Style_Theme_Design.md` | Style and theme system |
| `Animation_System_Design.md` | Animation and effects system |
| `Security_Performance_Design.md` | Security and performance optimization |

API documentation is in `docs/api/` covering individual controls and classes.

---

*For additional details, refer to the README.md and architecture documentation in docs/architecture/*
