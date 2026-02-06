# 示例项目结构

本文档展示一个使用LuaUI框架的典型项目结构。

## 项目目录结构

```
MyApp/
├── MyApp.exe                      # 主程序
├── LuaUI.dll                      # 框架核心
├── lua54.dll                      # Lua运行时
├── config.json                    # 应用配置
│
├── resources/                     # 资源文件
│   ├── app.theme                  # 应用级主题定义
│   ├── app.xaml                   # 应用级资源
│   │
│   ├── themes/                    # 主题目录
│   │   ├── default.theme          # 默认亮色主题
│   │   ├── dark.theme             # 暗色主题
│   │   └── high-contrast.theme    # 高对比度主题
│   │
│   ├── images/                    # 图片资源
│   │   ├── logo.png
│   │   ├── icons/
│   │   │   ├── new-file.png
│   │   │   ├── open-file.png
│   │   │   └── save-file.png
│   │   └── backgrounds/
│   │       └── splash.jpg
│   │
│   ├── fonts/                     # 字体文件
│   │   └── custom-font.ttf
│   │
│   └── sounds/                    # 音频资源
│       └── notification.wav
│
├── views/                         # 视图定义 (XML)
│   ├── main-window.xml            # 主窗口
│   ├── login-dialog.xml           # 登录对话框
│   │
│   ├── components/                # 可复用组件
│   │   ├── toolbar.xml
│   │   ├── sidebar.xml
│   │   └── status-bar.xml
│   │
│   └── dialogs/                   # 对话框
│       ├── about-dialog.xml
│       ├── settings-dialog.xml
│       └── confirm-dialog.xml
│
├── viewmodels/                    # 视图模型 (Lua)
│   ├── main-vm.lua                # 主窗口VM
│   ├── login-vm.lua               # 登录VM
│   │
│   ├── commands/                  # 命令定义
│   │   ├── file-commands.lua
│   │   └── edit-commands.lua
│   │
│   ├── converters/                # 值转换器
│   │   ├── date-converter.lua
│   │   └── boolean-converter.lua
│   │
│   └── services/                  # 服务层
│       ├── data-service.lua
│       ├── auth-service.lua
│       └── file-service.lua
│
├── models/                        # 数据模型 (Lua)
│   ├── user.lua
│   ├── document.lua
│   └── settings.lua
│
├── services/                      # C++服务扩展
│   ├── native-service.dll
│   └── plugin-loader.dll
│
├── plugins/                       # 插件
│   └── example-plugin/
│       ├── plugin.json
│       └── plugin.lua
│
├── data/                          # 应用数据
│   ├── settings.json              # 用户设置
│   ├── cache/                     # 缓存目录
│   └── logs/                      # 日志目录
│
└── locales/                       # 本地化
    ├── en.json                    # 英文
    ├── zh-CN.json                 # 简体中文
    └── ja.json                    # 日文
```

## 核心文件示例

### config.json
```json
{
    "app": {
        "name": "MyApp",
        "version": "1.0.0",
        "theme": "default",
        "language": "zh-CN",
        "singleInstance": true
    },
    "window": {
        "width": 1280,
        "height": 720,
        "minWidth": 800,
        "minHeight": 600,
        "centerScreen": true,
        "rememberPosition": true
    },
    "lua": {
        "maxMemoryMB": 128,
        "timeoutSeconds": 30,
        "enableDebug": false
    },
    "security": {
        "httpsOnly": true,
        "allowedDomains": ["api.example.com", "cdn.example.com"]
    }
}
```

### resources/app.xaml
```xml
<Application
    xmlns="http://schemas.luaui.org/2025/xaml"
    StartupUri="views/main-window.xml">
    
    <Application.Resources>
        <!-- 应用级资源 -->
        <SolidColorBrush x:Key="BrandColor" Color="#FF5722"/>
        <FontFamily x:Key="BrandFont">Custom Font, Segoe UI</FontFamily>
        
        <!-- 全局样式 -->
        <Style TargetType="Window">
            <Setter Property="Background" Value="{ThemeResource WindowBackgroundBrush}"/>
            <Setter Property="FontFamily" Value="{StaticResource BrandFont}"/>
        </Style>
        
        <!-- 转换器 -->
        <BooleanToVisibilityConverter x:Key="BoolToVis"/>
        <DateTimeConverter x:Key="DateConverter" Format="yyyy-MM-dd HH:mm"/>
    </Application.Resources>
    
</Application>
```

### views/main-window.xml
```xml
<Window x:Class="MyApp.MainWindow"
        xmlns="http://schemas.luaui.org/2025/xaml"
        xmlns:x="http://schemas.luaui.org/2025/xaml/x"
        Title="My Application"
        Width="1280" Height="720"
        WindowState="Maximized">
    
    <Window.DataContext>
        <x:LuaSource Path="viewmodels/main-vm.lua" Type="MainViewModel"/>
    </Window.DataContext>
    
    <DockPanel>
        <!-- 菜单栏 -->
        <Menu DockPanel.Dock="Top">
            <MenuItem Header="_File">
                <MenuItem Header="_New" Command="{Binding NewFileCommand}" 
                         InputGesture="Ctrl+N"/>
                <MenuItem Header="_Open" Command="{Binding OpenFileCommand}"
                         InputGesture="Ctrl+O"/>
                <Separator/>
                <MenuItem Header="_Exit" Command="{Binding ExitCommand}"/>
            </MenuItem>
            <MenuItem Header="_Edit">
                <MenuItem Header="_Undo" Command="{Binding UndoCommand}"
                         InputGesture="Ctrl+Z"/>
                <MenuItem Header="_Redo" Command="{Binding RedoCommand}"
                         InputGesture="Ctrl+Y"/>
            </MenuItem>
            <MenuItem Header="_Help">
                <MenuItem Header="_About" Command="{Binding ShowAboutCommand}"/>
            </MenuItem>
        </Menu>
        
        <!-- 工具栏 -->
        <ToolBar DockPanel.Dock="Top" Height="40">
            <Button Command="{Binding NewFileCommand}" 
                   ToolTip="New File (Ctrl+N)">
                <Image Source="resources/images/icons/new-file.png" Width="16"/>
            </Button>
            <Button Command="{Binding OpenFileCommand}"
                   ToolTip="Open File (Ctrl+O)">
                <Image Source="resources/images/icons/open-file.png" Width="16"/>
            </Button>
            <Separator/>
            <Button Command="{Binding SaveFileCommand}"
                   ToolTip="Save (Ctrl+S)">
                <Image Source="resources/images/icons/save-file.png" Width="16"/>
            </Button>
        </ToolBar>
        
        <!-- 侧边栏 -->
        <Border DockPanel.Dock="Left" Width="250"
               Background="{ThemeResource SurfaceBrush}">
            <TreeView ItemsSource="{Binding ProjectItems}"
                     SelectedItem="{Binding SelectedItem, Mode=TwoWay}">
                <TreeView.ItemTemplate>
                    <DataTemplate>
                        <StackPanel Orientation="Horizontal">
                            <Image Source="{Binding Icon}" Width="16" Height="16"/>
                            <TextBlock Text="{Binding Name}" Margin="8,0"/>
                        </StackPanel>
                    </DataTemplate>
                </TreeView.ItemTemplate>
            </TreeView>
        </Border>
        
        <!-- 主内容区 -->
        <Grid>
            <Grid.RowDefinitions>
                <RowDefinition Height="*"/>
                <RowDefinition Height="Auto"/>
            </Grid.RowDefinitions>
            
            <!-- 内容 -->
            <ContentControl Content="{Binding CurrentView}"
                          ContentTemplateSelector="{Binding ViewSelector}"/>
            
            <!-- 状态栏 -->
            <StatusBar Grid.Row="1">
                <TextBlock Text="{Binding StatusMessage}"/>
                <StatusBarItem HorizontalAlignment="Right">
                    <ProgressBar Value="{Binding ProgressValue}"
                               Maximum="{Binding ProgressMaximum}"
                               Width="100" Height="16"
                               IsVisible="{Binding IsProgressVisible}"/>
                </StatusBarItem>
            </StatusBar>
        </Grid>
    </DockPanel>
    
</Window>
```

### viewmodels/main-vm.lua
```lua
-- viewmodels/main-vm.lua
local ViewModel = require("UI.ViewModel")
local ObservableCollection = require("UI.ObservableCollection")
local Command = require("UI.Command")
local AsyncCommand = require("UI.AsyncCommand")

local MainViewModel = setmetatable({}, {__index = ViewModel})
MainViewModel.__index = MainViewModel

function MainViewModel.new()
    local self = setmetatable(ViewModel.new(), MainViewModel)
    
    -- 属性
    self._windowTitle = "My Application"
    self._statusMessage = "Ready"
    self._isProgressVisible = false
    self._progressValue = 0
    self._progressMaximum = 100
    self._projectItems = ObservableCollection.new()
    self._selectedItem = nil
    self._currentView = nil
    self._canUndo = false
    self._canRedo = false
    
    -- 命令
    self.NewFileCommand = Command.new(
        function() self:onNewFile() end,
        function() return true end
    )
    
    self.OpenFileCommand = Command.new(
        function() self:onOpenFile() end
    )
    
    self.SaveFileCommand = Command.new(
        function() self:onSaveFile() end,
        function() return self._currentDocument ~= nil end
    )
    
    self.ExitCommand = Command.new(
        function() self:onExit() end
    )
    
    self.LoadDataCommand = AsyncCommand.new(
        function() return self:loadDataAsync() end,
        function() return not self._isLoading end
    )
    
    -- 初始化
    self:initialize()
    
    return self
end

-- 属性访问器
function MainViewModel:getWindowTitle()
    return self._windowTitle
end

function MainViewModel:setWindowTitle(value)
    if self._windowTitle ~= value then
        self._windowTitle = value
        self:notifyPropertyChanged("WindowTitle")
    end
end

function MainViewModel:getStatusMessage()
    return self._statusMessage
end

function MainViewModel:setStatusMessage(value)
    if self._statusMessage ~= value then
        self._statusMessage = value
        self:notifyPropertyChanged("StatusMessage")
    end
end

function MainViewModel:getIsProgressVisible()
    return self._isProgressVisible
end

function MainViewModel:setIsProgressVisible(value)
    if self._isProgressVisible ~= value then
        self._isProgressVisible = value
        self:notifyPropertyChanged("IsProgressVisible")
    end
end

function MainViewModel:getProjectItems()
    return self._projectItems
end

function MainViewModel:getSelectedItem()
    return self._selectedItem
end

function MainViewModel:setSelectedItem(value)
    if self._selectedItem ~= value then
        self._selectedItem = value
        self:notifyPropertyChanged("SelectedItem")
        -- 触发其他命令状态更新
        self.SaveFileCommand:raiseCanExecuteChanged()
    end
end

-- 命令处理
function MainViewModel:onNewFile()
    local dialog = UI.SaveFileDialog.new({
        title = "New File",
        filter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*",
        defaultExt = "txt"
    })
    
    if dialog:showDialog() then
        self:createNewFile(dialog.fileName)
    end
end

function MainViewModel:onOpenFile()
    local dialog = UI.OpenFileDialog.new({
        title = "Open File",
        filter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*",
        multiSelect = false
    })
    
    if dialog:showDialog() then
        self:loadFile(dialog.fileName)
    end
end

function MainViewModel:onSaveFile()
    if self._currentDocument then
        self:saveDocument(self._currentDocument)
    end
end

function MainViewModel:onExit()
    -- 检查是否有未保存的更改
    if self:hasUnsavedChanges() then
        local result = UI.showConfirm(
            "You have unsaved changes. Save before exit?",
            "Confirm Exit",
            UI.MessageIcon.Question
        )
        
        if result == UI.DialogResult.Yes then
            self:onSaveFile()
        elseif result == UI.DialogResult.Cancel then
            return
        end
    end
    
    UI.exit(0)
end

-- 异步加载
function MainViewModel:loadDataAsync()
    self:setIsProgressVisible(true)
    self:setStatusMessage("Loading data...")
    
    return Task.run(function(progress)
        -- 模拟加载
        for i = 1, 10 do
            Task.delay(100)
            progress:report(i / 10)
        end
        
        -- 返回数据
        return {
            { id = 1, name = "Project A", icon = "resources/images/icons/folder.png" },
            { id = 2, name = "Project B", icon = "resources/images/icons/folder.png" },
            { id = 3, name = "File.txt", icon = "resources/images/icons/file.png" },
        }
    end):continueOnUI(function(data)
        self._projectItems:clear()
        for _, item in ipairs(data) do
            self._projectItems:add(item)
        end
        self:setIsProgressVisible(false)
        self:setStatusMessage("Ready")
    end)
end

-- 初始化
function MainViewModel:initialize()
    -- 加载设置
    local settings = Storage.Settings.get()
    self:setWindowTitle(settings:get("window.title", "My Application"))
    
    -- 加载初始数据
    self.LoadDataCommand:execute()
end

-- 生命周期
function MainViewModel:onActivated()
    Logger.info("MainViewModel activated")
end

function MainViewModel:onDeactivated()
    Logger.info("MainViewModel deactivated")
end

return MainViewModel
```

### models/user.lua
```lua
-- models/user.lua
local User = {}
User.__index = User

function User.new(data)
    local self = setmetatable({}, User)
    
    self.id = data.id or 0
    self.name = data.name or ""
    self.email = data.email or ""
    self.avatar = data.avatar or nil
    self.createdAt = data.createdAt or os.time()
    
    return self
end

function User:validate()
    local errors = {}
    
    if #self.name < 2 then
        table.insert(errors, "Name must be at least 2 characters")
    end
    
    if not self.email:match("^[%w.+-]+@[%w.-]+%.%w%w+$") then
        table.insert(errors, "Invalid email format")
    end
    
    return #errors == 0, errors
end

function User:toJSON()
    return {
        id = self.id,
        name = self.name,
        email = self.email,
        avatar = self.avatar,
        createdAt = self.createdAt
    }
end

function User.fromJSON(json)
    return User.new(json)
end

return User
```

### 构建脚本
```powershell
# build.ps1
param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

# 清理输出目录
Remove-Item -Path "build" -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path "build" | Out-Null

# 复制依赖
Copy-Item "libs/LuaUI.dll" "build/"
Copy-Item "libs/lua54.dll" "build/"

# 复制资源
Copy-Item -Path "resources" -Destination "build/" -Recurse
Copy-Item -Path "views" -Destination "build/" -Recurse
Copy-Item -Path "viewmodels" -Destination "build/" -Recurse
Copy-Item -Path "models" -Destination "build/" -Recurse
Copy-Item -Path "locales" -Destination "build/" -Recurse

# 复制配置
Copy-Item "config.json" "build/"

# 编译主程序（如果有C++代码）
# msbuild MyApp.sln /p:Configuration=$Configuration /p:Platform=$Platform

# 复制可执行文件
Copy-Item "bin/$Configuration/MyApp.exe" "build/"

Write-Host "Build complete: build/"
```

## 开发工作流

### 1. 创建新视图
```
1. 在 views/ 目录创建 .xml 文件
2. 在 viewmodels/ 目录创建对应的 .lua 文件
3. 在 app.xaml 注册导航（如果需要）
4. 在主ViewModel添加导航命令
```

### 2. 添加新控件
```
1. 在 views/ 中使用 XML 定义控件布局
2. 在 app.xaml 或 theme 中添加控件样式
3. 在 viewmodels/ 中处理控件交互逻辑
```

### 3. 热重载开发
```lua
-- 开启热重载监听
UI.enableHotReload(true)

-- 监听文件变化
UI.onFileChanged(function(file)
    Logger.info("File changed: " .. file)
    -- 自动重载视图
    if file:match("%.xml$") then
        UI.reloadView(file)
    elseif file:match("%.lua$") then
        UI.reloadScript(file)
    end
end)
```
