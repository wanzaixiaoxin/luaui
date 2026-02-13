# LuaUI Project - AI Coding Agent Guide

## Project Overview

**LuaUI** is a commercial-grade Windows UI framework with the following key characteristics:

- **Rendering**: Hardware-accelerated 2D graphics using Direct2D/DirectWrite
- **Layout System**: Declarative XML layouts (XAML-like) with flexible layout containers
- **Data Binding**: Full MVVM (Model-View-ViewModel) support with OneWay, TwoWay bindings
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
| Scripting | Lua 5.4 (headers included) |
| Windows API | Win32, DXGI |

## Project Structure

```
luaui/
├── CMakeLists.txt              # Root CMake configuration
├── src/luaui/                  # Main source code
│   ├── core/                   # Core framework (Window, Control, components)
│   ├── controls/               # UI controls (Button, TextBox, etc.)
│   │   └── layouts/            # Layout containers (StackPanel, Grid, etc.)
│   ├── rendering/              # Rendering engine interfaces + Direct2D impl
│   │   └── d2d/                # Direct2D implementation
│   ├── xml/                    # XML layout parsing
│   ├── mvvm/                   # MVVM data binding system
│   ├── style/                  # Style system (Theme, Style, ResourceDictionary)
│   └── utils/                  # Utilities (Logger, etc.)
├── third_party/                # External dependencies
│   ├── lua/                    # Lua 5.4 headers
│   └── tinyxml2/               # XML parsing library
├── examples/                   # Example applications (01_basic_window ~ 13_mvvm_demo)
├── tests/                      # Unit tests with custom test framework
├── docs/                       # Architecture documentation (mostly in Chinese)
├── resources/                  # Framework resources
└── libs/                       # Pre-built Lua binaries (lua.exe, lua54.dll)
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
| `LUAUI_BUILD_OLD_EXAMPLES` | OFF | Build legacy examples (06-12) |

### Output Directories

- Executables: `${CMAKE_BINARY_DIR}/bin/`
- Libraries: `${CMAKE_BINARY_DIR}/lib/`

## Module Architecture

The project uses a modular CMake structure with interface libraries:

```
LuaUI (Interface Library)
├── LuaUI_Core        # Window, Control, Components, Interfaces
├── LuaUI_Controls    # Button, TextBox, StackPanel, Grid, etc.
├── LuaUI_Rendering   # IRenderContext + Direct2D implementation
├── LuaUI_Style       # Theme, Style, ResourceDictionary
├── LuaUI_Xml         # XML layout loader
└── LuaUI_MVVM        # Data binding engine
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
| `ASSERT_EQ(expected, actual)` | Assert equality |
| `ASSERT_NE(a, b)` | Assert not equal |
| `ASSERT_FLOAT_EQ(e, a, epsilon)` | Assert float equality with tolerance |
| `ASSERT_NEAR(e, a, epsilon)` | Same as ASSERT_FLOAT_EQ |
| `ASSERT_NULL(ptr)` | Assert pointer is null |
| `ASSERT_NOT_NULL(ptr)` | Assert pointer is not null |
| `ASSERT_THROW(expr, type)` | Assert exception is thrown |
| `ASSERT_NO_THROW(expr)` | Assert no exception thrown |

## Key Design Patterns

### 1. Component-Based Architecture

Controls use a component system to separate concerns:

```cpp
class Control {
protected:
    virtual void InitializeComponents();  // Override to add components
    
private:
    components::ComponentHolder m_components;
    // LayoutComponent - handles layout
    // RenderComponent - handles rendering
    // InputComponent - handles input
};
```

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
- `IControl` - Basic control operations
- `IRenderable` - Rendering capability
- `ILayoutable` - Layout capability
- `IInputHandler` - Input handling
- `IStyleable` - Styling support

## Common Development Tasks

### Adding a New Control

1. Create header/implementation in `src/luaui/controls/`
2. Inherit from `Control` or `Panel`
3. Override `InitializeComponents()`, `OnRender()`, `OnMeasure()`
4. Add to `src/luaui/controls/CMakeLists.txt`
5. Export in `src/luaui/controls/Controls.h` if needed
6. Create example in `examples/`

### Adding a New Layout Panel

1. Create files in `src/luaui/controls/layouts/`
2. Inherit from `Panel` (which has `PanelLayoutComponent`)
3. Implement custom layout logic in `OnArrange()`
4. Add to `layouts/CMakeLists.txt`
5. Export in `layouts/Layouts.h`

### Adding XML Support for a Control

1. The `XmlLoader` uses `TypeConverter` for attribute parsing
2. Add type conversion in `src/luaui/xml/TypeConverter.cpp`
3. For complex controls, may need to extend XML loader

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

### Documentation Language

Architecture documentation in `docs/architecture/` is primarily in **Chinese**.
Key files:
- `docs/architecture/README.md` - Architecture overview
- `docs/architecture/MVVM_DataBinding_Design.md` - MVVM design (Chinese)
- `docs/architecture/Render_Engine_Design.md` - Rendering design (Chinese)

## Security Considerations

- No sandboxing for Lua scripts is currently implemented (planned for Phase 4)
- XML layouts can reference external resources - validate paths
- No HTTPS/network security features currently active

## Examples Reference

| Example | Description |
|---------|-------------|
| 01_basic_window | Win32 + Direct2D basics |
| 02_hello_lua | Lua integration |
| 03_rendering_demo | Rendering engine |
| 04_animation_demo | Animation system |
| 05_offscreen_demo | Offscreen rendering |
| 06_controls_demo | Control system |
| 08_visual_layout_demo | Visual layout |
| 12_xml_layout_demo | XML layouts with code-behind |
| 13_mvvm_demo | **MVVM data binding demo** |

Run examples from build directory:
```bash
./bin/Release/13_mvvm_demo.exe
```

---

*For additional details, refer to the README.md and architecture documentation in docs/architecture/.*
