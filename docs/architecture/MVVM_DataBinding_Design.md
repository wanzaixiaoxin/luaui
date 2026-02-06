# MVVM 数据绑定系统设计

## 1. 概述

MVVM（Model-View-ViewModel）是一种架构模式，将UI逻辑与业务逻辑分离，通过数据绑定实现自动同步。

## 2. 架构图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           MVVM 架构                                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌─────────────┐         ┌─────────────┐         ┌─────────────┐          │
│   │    View     │ ◄─────► │  ViewModel  │ ◄─────► │    Model    │          │
│   │             │         │             │         │             │          │
│   │  - XML定义   │  绑定    │  - Lua实现   │  调用    │  - 数据结构  │          │
│   │  - 控件树    │         │  - 业务逻辑  │         │  - 持久化    │          │
│   │  - 事件处理  │         │  - 命令      │         │  - 验证      │          │
│   │  - 动画触发  │         │  - 属性通知  │         │  - 转换      │          │
│   └─────────────┘         └─────────────┘         └─────────────┘          │
│          │                         │                                        │
│          │                         │                                        │
│          ▼                         ▼                                        │
│   ┌─────────────────────────────────────────────────────────────────┐     │
│   │                        Binding Engine                            │     │
│   │                                                                  │     │
│   │  ┌─────────────────────────────────────────────────────────┐   │     │
│   │  │                     绑定类型                              │   │     │
│   │  │  • OneWay      : 单向绑定 (VM → View)                    │   │     │
│   │  │  • TwoWay      : 双向绑定 (VM ↔ View)                    │   │     │
│   │  │  • OneWayToSource : 单向到源 (View → VM)                 │   │     │
│   │  │  • OneTime     : 一次性绑定                             │   │     │
│   │  └─────────────────────────────────────────────────────────┘   │     │
│   │                                                                  │     │
│   │  ┌─────────────────────────────────────────────────────────┐   │     │
│   │  │                     核心功能                              │   │     │
│   │  │  • 属性变更通知 (INotifyPropertyChanged)                 │   │     │
│   │  │  • 集合同步 (INotifyCollectionChanged)                   │   │     │
│   │  │  • 值转换器 (IValueConverter)                            │   │     │
│   │  │  • 数据验证 (IDataErrorInfo)                             │   │     │
│   │  │  • 延迟加载 (Lazy Loading)                               │   │     │
│   │  │  • 绑定表达式解析                                        │   │     │
│   │  └─────────────────────────────────────────────────────────┘   │     │
│   │                                                                  │     │
│   └─────────────────────────────────────────────────────────────────┘     │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 3. 核心类设计

见代码文件中的详细设计。

## 4. XML绑定语法示例

```xml
<Window xmlns="http://schemas.luaui.org/2025/xaml"
        DataContext="{Binding Source={x:LuaSource Path='viewmodels/main_vm.lua', Type='MainViewModel'}}">
    
    <!-- 简单绑定 -->
    <TextBlock Text="{Binding UserName}" />
    
    <!-- 双向绑定 -->
    <TextBox Text="{Binding UserName, Mode=TwoWay, UpdateSourceTrigger=PropertyChanged}" />
    
    <!-- 带转换器的绑定 -->
    <TextBlock Text="{Binding Birthday, Converter={StaticResource DateTimeConverter}, ConverterParameter='yyyy-MM-dd'}" />
    <TextBlock Visibility="{Binding IsLoading, Converter={StaticResource BooleanToVisibilityConverter}}" 
               Text="Loading..." />
    
    <!-- 绑定到元素本身 -->
    <TextBlock Text="{Binding RelativeSource={RelativeSource Self}, Path=Name}" />
    
    <!-- 绑定到父元素 -->
    <TextBlock Text="{Binding RelativeSource={RelativeSource AncestorType=Window}, Path=Title}" />
    
    <!-- 绑定到命名元素 -->
    <Slider x:Name="zoomSlider" Minimum="0.5" Maximum="3" Value="1" />
    <Image Source="{Binding ImageSource}" 
           ScaleX="{Binding ElementName=zoomSlider, Path=Value}"
           ScaleY="{Binding ElementName=zoomSlider, Path=Value}" />
    
    <!-- 多绑定 -->
    <TextBlock>
        <TextBlock.Text>
            <MultiBinding Converter="{StaticResource FullNameConverter}">
                <Binding Path="FirstName" />
                <Binding Path="LastName" />
            </MultiBinding>
        </TextBlock.Text>
    </TextBlock>
    
    <!-- 命令绑定 -->
    <Button Content="Save" Command="{Binding SaveCommand}" 
            CommandParameter="{Binding SelectedItem}" />
    
    <!-- 异步命令（自动显示忙碌状态） -->
    <Button Content="Load Data">
        <Button.Command>
            <AsyncCommand Execute="{Binding LoadDataAsync}" />
        </Button.Command>
    </Button>
    
    <!-- 集合绑定 -->
    <ListBox ItemsSource="{Binding Users}" SelectedItem="{Binding SelectedUser, Mode=TwoWay}">
        <ListBox.ItemTemplate>
            <DataTemplate>
                <StackPanel>
                    <TextBlock Text="{Binding Name}" FontWeight="Bold" />
                    <TextBlock Text="{Binding Email}" Foreground="Gray" />
                </StackPanel>
            </DataTemplate>
        </ListBox.ItemTemplate>
    </ListBox>
    
    <!-- 带验证的绑定 -->
    <TextBox>
        <TextBox.Text>
            <Binding Path="Age" Mode="TwoWay">
                <Binding.ValidationRules>
                    <RangeValidationRule Min="0" Max="150" />
                    <IntegerValidationRule />
                </Binding.ValidationRules>
            </Binding>
        </TextBox.Text>
    </TextBox>
    
</Window>
```

## 5. Lua ViewModel实现示例

```lua
-- MainViewModel.lua
local BaseViewModel = require("UI.BaseViewModel")
local Command = require("UI.Command")
local AsyncCommand = require("UI.AsyncCommand")
local ObservableCollection = require("UI.ObservableCollection")

local MainViewModel = setmetatable({}, {__index = BaseViewModel})
MainViewModel.__index = MainViewModel

function MainViewModel.new()
    local self = setmetatable(BaseViewModel.new(), MainViewModel)
    
    -- 定义属性
    self._windowTitle = "My Application"
    self._userName = ""
    self._isLoading = false
    self._users = ObservableCollection.new()
    self._selectedUser = nil
    
    -- 定义命令
    self.LoadUsersCommand = AsyncCommand.new(
        function() return self:loadUsersAsync() end,
        function() return not self._isLoading end
    )
    
    self.SaveUserCommand = Command.new(
        function() self:saveUser() end,
        function() return self._selectedUser ~= nil end
    )
    
    return self
end

-- 属性 getter/setter
function MainViewModel:getWindowTitle()
    return self._windowTitle
end

function MainViewModel:setWindowTitle(value)
    if self._windowTitle ~= value then
        self._windowTitle = value
        self:notifyPropertyChanged("WindowTitle")
    end
end

function MainViewModel:getUserName()
    return self._userName
end

function MainViewModel:setUserName(value)
    if self._userName ~= value then
        self._userName = value
        self:notifyPropertyChanged("UserName")
    end
end

function MainViewModel:getIsLoading()
    return self._isLoading
end

function MainViewModel:setIsLoading(value)
    if self._isLoading ~= value then
        self._isLoading = value
        self:notifyPropertyChanged("IsLoading")
    end
end

function MainViewModel:getUsers()
    return self._users
end

function MainViewModel:getSelectedUser()
    return self._selectedUser
end

function MainViewModel:setSelectedUser(value)
    if self._selectedUser ~= value then
        self._selectedUser = value
        self:notifyPropertyChanged("SelectedUser")
        self.SaveUserCommand:raiseCanExecuteChanged()
    end
end

-- 异步方法
function MainViewModel:loadUsersAsync()
    self:setIsLoading(true)
    
    return Task.run(function()
        Task.delay(1000)
        return {
            { id = 1, name = "Alice", email = "alice@example.com" },
            { id = 2, name = "Bob", email = "bob@example.com" },
        }
    end):continueOnUI(function(users)
        self._users:clear()
        for _, user in ipairs(users) do
            self._users:add(user)
        end
        self:setIsLoading(false)
    end)
end

function MainViewModel:saveUser()
    if not self._selectedUser then return end
    Logger.info("Saving user: " .. self._selectedUser.name)
end

return MainViewModel
```

## 6. 性能优化

1. **批量更新**: 使用BeginUpdate/EndUpdate减少通知次数
2. **延迟绑定**: 不在可视区域的绑定暂停更新
3. **弱引用**: 避免ViewModel和View之间的循环引用
4. **缓存**: 缓存绑定表达式解析结果
5. **虚拟化**: 集合控件使用虚拟化减少内存占用
