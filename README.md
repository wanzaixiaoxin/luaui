# LuaUI

A Windows UI framework with Lua scripting and Direct2D rendering.

## Project Structure

```
luaui/
├── CMakeLists.txt          # Main build configuration
├── README.md               # This file
├── LICENSE                 # License file
├── third_party/            # Third-party libraries
│   ├── lua/               # Lua headers
│   └── tinyxml2/          # TinyXML2 library
├── src/                    # Source code (to be implemented)
│   └── luaui/
│       ├── core/          # Core functionality
│       ├── rendering/     # Direct2D rendering
│       ├── ui/            # UI controls
│       ├── scripting/     # Lua integration
│       └── utils/         # Utilities
├── examples/              # Example applications
│   ├── 01_basic_window.cpp
│   └── 02_hello_lua.cpp
└── tests/                 # Unit tests
    └── simple_test.cpp
```

## Build Instructions

```bash
# Configure
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"

# Build
cmake --build . --config Release
```

## Requirements

- Windows 10/11
- Visual Studio 2022 (MSVC 19.44+)
- CMake 3.15+

## Dependencies

- **Lua**: Embedded scripting (headers in third_party/lua)
- **TinyXML2**: XML parsing (source in third_party/tinyxml2)
- **Direct2D**: Windows built-in 2D graphics
