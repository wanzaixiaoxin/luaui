# LuaUI

A Windows UI framework with Lua scripting and Direct2D rendering.

## Features

- **Direct2D Rendering** - Hardware-accelerated 2D graphics
- **Layout System** - StackPanel, Grid, Canvas, DockPanel, WrapPanel
- **Control System** - Button, TextBox, CheckBox, Slider, ProgressBar, etc.
- **Lua Scripting** - Embed Lua for UI logic
- **Modern C++** - C++17 standard

## Project Structure

```
src/luaui/
├── controls/           # UI Controls
│   ├── include/        # Control.h, Event.h, Shapes.h...
│   ├── layout/         # Layout containers
│   │   ├── include/    # StackPanel.h, Grid.h, Canvas.h...
│   │   └── src/
│   └── src/            # Control implementations
├── rendering/          # Rendering engine
│   ├── include/        # IRenderContext.h, Types.h...
│   └── src/d2d/        # Direct2D implementation
└── utils/              # Utilities
    └── include/        # Logger.h
```

## Quick Start

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Run Examples

```bash
# Layout demo
./bin/Release/07_layout_demo.exe

# Visual layout demo (GUI)
./bin/Release/08_visual_layout_demo.exe

# Composite layout demo
./bin/Release/09_composite_layout.exe
```

### Use Layout System

```cpp
#include "Control.h"
#include "layout.h"

// Create vertical stack panel
auto panel = std::make_shared<StackPanel>();
panel->SetOrientation(StackPanel::Orientation::Vertical);
panel->SetSpacing(10);

// Add child controls
auto button = std::make_shared<Button>();
panel->AddChild(button);

// Measure and arrange
panel->Measure(Size(800, 600));
panel->Arrange(Rect(0, 0, 800, 600));
```

## Documentation

- [Directory Structure](docs/architecture/DIRECTORY_STRUCTURE.md)
- [Layout System](docs/architecture/Layout_System.md)
- [Architecture Overview](docs/architecture/README.md)

## Examples

| Example | Description |
|---------|-------------|
| 01_basic_window | Win32 + Direct2D basics |
| 02_hello_lua | Lua integration |
| 03_rendering_demo | Rendering engine demo |
| 04_animation_demo | Animation system |
| 05_offscreen_demo | Offscreen rendering |
| 06_controls_demo | Control system |
| 07_layout_demo | Layout engine |
| 08_visual_layout_demo | Visual layout showcase |
| 09_composite_layout | Layout + Controls composite |

## License

MIT License - see [LICENSE](LICENSE) file
