# LuaUI

A Windows UI framework with Direct2D rendering, MVVM data binding, and declarative XML layouts.

## Features

- **Direct2D Rendering** - Hardware-accelerated 2D graphics
- **MVVM Data Binding** - Declarative bindings with TwoWay support
- **Layout System** - StackPanel, Grid, Canvas, DockPanel, WrapPanel
- **Control System** - Button, TextBox, CheckBox, RadioButton, Slider, ProgressBar, Image, etc.
- **XML Layout System** - Declarative UI with external config files and code-behind
- **Image Support** - Load and display images with multiple stretch modes
- **Modern C++** - C++17 standard

## Project Structure

```
src/luaui/
├── controls/           # UI Controls
│   ├── Button.h/cpp
│   ├── TextBlock.h/cpp
│   ├── TextBox.h/cpp
│   ├── CheckBox.h/cpp      # Includes RadioButton
│   ├── Slider.h/cpp
│   ├── ProgressBar.h/cpp
│   ├── Image.h/cpp         # Image display with stretch modes
│   ├── ListBox.h/cpp
│   └── layouts/            # Layout containers
│       ├── StackPanel.h/cpp
│       ├── Grid.h/cpp
│       ├── Canvas.h/cpp
│       └── ...
├── core/               # Core framework
│   ├── Components/         # Component-based architecture
│   │   ├── LayoutComponent.h/cpp
│   │   ├── RenderComponent.h/cpp
│   │   └── InputComponent.h/cpp
│   ├── Control.h/cpp
│   └── Window.h/cpp
├── rendering/          # Rendering engine
│   ├── IRenderContext.h    # Rendering interface
│   ├── IBitmap.h           # Bitmap/image support
│   ├── Types.h
│   └── d2d/                # Direct2D implementation
├── xml/                # XML Layout system
│   ├── XmlLoader.cpp
│   └── ...
├── mvvm/               # MVVM Data Binding
│   ├── MvvmXmlLoader.h/cpp     # MVVM-enabled XML loader
│   ├── BindingEngine.h/cpp     # Binding expression parser
│   ├── ViewModelBase.h         # INotifyPropertyChanged base
│   └── Converters.h            # Value converters
└── utils/              # Utilities
    └── Logger.h
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
# MVVM Data Binding Demo
./bin/Release/13_mvvm_demo.exe

# Layout demo
./bin/Release/07_layout_demo.exe

# Visual layout demo (GUI)
./bin/Release/08_visual_layout_demo.exe

# Composite layout demo
./bin/Release/09_composite_layout.exe
```

### MVVM Data Binding

```xml
<!-- MVVM Layout with Data Binding -->
<StackPanel xmlns="http://luaui.io/schema">
    <!-- OneWay Binding -->
    <TextBlock Text="{Binding Status}" FontSize="14"/>
    
    <!-- TwoWay Binding -->
    <TextBox Text="{Binding UserName, Mode=TwoWay}" Width="300"/>
    <Slider Value="{Binding Progress, Mode=TwoWay}" Minimum="0" Maximum="100"/>
    <CheckBox IsChecked="{Binding IsPremium, Mode=TwoWay}" Text="Premium Member"/>
    
    <!-- Converter -->
    <TextBlock Text="{Binding Progress, Converter=Format, ConverterParameter='Progress: {0}%'})"/>
    
    <!-- Image with Stretch mode -->
    <Image SourcePath="image.png" Width="100" Height="100" Stretch="Uniform"/>
    
    <!-- RadioButton Group -->
    <StackPanel Orientation="Horizontal">
        <RadioButton Name="rbMale" Text="Male" GroupName="Gender" IsChecked="True"/>
        <RadioButton Name="rbFemale" Text="Female" GroupName="Gender"/>
    </StackPanel>
</StackPanel>
```

```cpp
#include "MainWindow.h"
#include "UserViewModel.h"

class MainWindow : public luaui::Window {
public:
    void OnLoaded() override {
        // Create ViewModel
        m_viewModel = std::make_shared<UserViewModel>();
        m_viewModel->SetUserName(L"John Doe");
        m_viewModel->SetProgress(65);
        
        // Load MVVM layout
        auto loader = luaui::mvvm::CreateMvvmXmlLoader();
        auto root = loader->Load("layout.xml");
        
        // Set DataContext - triggers all bindings
        loader->SetDataContext(m_viewModel);
        
        SetRoot(root);
    }
    
private:
    std::shared_ptr<UserViewModel> m_viewModel;
};
```

### ViewModel Implementation

```cpp
class UserViewModel : public luaui::mvvm::ViewModelBase {
public:
    std::wstring GetUserName() const { return m_userName; }
    void SetUserName(const std::wstring& value) {
        if (m_userName != value) {
            m_userName = value;
            RaisePropertyChanged("UserName");
        }
    }
    
    int GetProgress() const { return m_progress; }
    void SetProgress(int value) {
        if (m_progress != value) {
            m_progress = value;
            RaisePropertyChanged("Progress");
        }
    }
    
private:
    std::wstring m_userName;
    int m_progress = 0;
};
```

### Image Control

```xml
<!-- Image with different stretch modes -->
<Image SourcePath="photo.png" Width="200" Height="150" Stretch="Uniform"/>
<Image SourcePath="icon.png" Width="32" Height="32" Stretch="None"/>
<Image SourcePath="background.jpg" Stretch="UniformToFill"/>
```

**Stretch Modes:**
- `None` - Keep original size, centered
- `Fill` - Fill entire area (may distort)
- `Uniform` - Scale proportionally (default)
- `UniformToFill` - Fill area, may clip

### Button with State Colors

```xml
<Button Text="Save" Width="90" Height="32" Background="#22C55E"/>
<Button Text="Cancel" Width="90" Height="32" Background="#EF4444"/>
```

Button automatically generates:
- Hover state: 15% lighter
- Pressed state: 15% darker

## Documentation

- [Directory Structure](docs/architecture/DIRECTORY_STRUCTURE.md)
- [Layout System](docs/architecture/Layout_System.md)
- [Architecture Overview](docs/architecture/README.md)

## Examples

| Example | Description |
|---------|-------------|
| 13_mvvm_demo | **MVVM Data Binding Demo** - Declarative bindings, converters, TwoWay binding |
| 12_xml_layout_demo | XML Layout with code-behind |
| 09_composite_layout | Layout + Controls composite |
| 08_visual_layout_demo | Visual layout showcase |
| 07_layout_demo | Layout engine |
| 06_controls_demo | Control system |
| 05_offscreen_demo | Offscreen rendering |
| 04_animation_demo | Animation system |
| 03_rendering_demo | Rendering engine demo |
| 02_hello_lua | Lua integration |
| 01_basic_window | Win32 + Direct2D basics |

## License

MIT License - see [LICENSE](LICENSE) file
