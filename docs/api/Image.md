# Image 图片控件

`Image` 用于显示图像资源，支持多种图片格式、缩放模式和异步加载。

## 基本用法

### XML 中使用

```xml
<!-- 基本用法 -->
<Image Source="assets/logo.png"/>

<!-- 指定尺寸 -->
<Image Source="assets/logo.png" Width="200" Height="100"/>

<!-- 缩放模式 -->
<Image Source="assets/photo.jpg" Stretch="Uniform"/>

<!-- 绑定图片源 -->
<Image Source="{Binding AvatarPath}"/>

<!-- 异步加载（大图片） -->
<Image Source="assets/large_image.jpg" IsAsync="true"/>
```

### 代码中使用

```cpp
#include "controls/Image.h"

// 创建图片控件
auto image = std::make_shared<luaui::controls::Image>();

// 设置图片源
image->SetSource(L"assets/logo.png");

// 设置尺寸
image->SetWidth(200);
image->SetHeight(100);

// 设置缩放模式
image->SetStretch(luaui::controls::Stretch::Uniform);

// 异步加载
image->SetSourceAsync(L"assets/large_image.jpg", []() {
    // 加载完成回调
    std::cout << "图片加载完成" << std::endl;
});
```

## 属性

| 属性 | 类型 | 说明 | XML 属性 | 默认值 |
|------|------|------|----------|--------|
| `Source` | string/ImageSource | 图片源路径或资源 | `Source` | null |
| `Stretch` | Stretch | 缩放模式 | `Stretch` | Uniform |
| `IsAsync` | bool | 是否异步加载 | `IsAsync` | false |
| `DecodeWidth` | int | 解码宽度 | `DecodeWidth` | 0（原始尺寸） |
| `DecodeHeight` | int | 解码高度 | `DecodeHeight` | 0（原始尺寸） |

### Stretch 缩放模式

| 模式 | 说明 | 适用场景 |
|------|------|----------|
| `None` | 不缩放，按原始尺寸显示 | 需要精确像素显示 |
| `Fill` | 拉伸填满，可能变形 | 固定尺寸容器 |
| `Uniform` | 等比缩放，完整显示 | 默认，保持比例 |
| `UniformToFill` | 等比缩放，填满裁剪 | 背景图、封面 |

```xml
<!-- 等比缩放 -->
<Image Source="photo.jpg" Stretch="Uniform" Width="200" Height="200"/>

<!-- 填满裁剪（可能裁剪部分内容） -->
<Image Source="background.jpg" Stretch="UniformToFill"/>

<!-- 原始尺寸 -->
<Image Source="icon.png" Stretch="None"/>
```

## 使用场景

### 头像显示

```xml
<Border CornerRadius="50" Width="100" Height="100" ClipToBounds="true">
    <Image Source="{Binding AvatarUrl}" 
           Stretch="UniformToFill"
           IsAsync="true"/>
</Border>

<!-- 带占位符 -->
<Grid Width="100" Height="100">
    <!-- 加载中的占位符 -->
    <Border Background="#F0F0F0" 
            IsVisible="{Binding IsAvatarLoading}"/>
    
    <!-- 图片 -->
    <Image Source="{Binding AvatarUrl}" 
           Stretch="UniformToFill"
           IsAsync="true"/>
</Grid>
```

### 图片列表

```xml
<ScrollViewer>
    <WrapPanel Orientation="Horizontal" ItemWidth="200" ItemHeight="150">
        <ItemsControl ItemsSource="{Binding Photos}">
            <ItemsControl.ItemTemplate>
                <DataTemplate>
                    <Border CornerRadius="8" 
                            Margin="8"
                            ClipToBounds="true">
                        <Image Source="{Binding ThumbnailPath}"
                               Stretch="UniformToFill"
                               ToolTip="{Binding FileName}"/>
                    </Border>
                </DataTemplate>
            </ItemsControl.ItemTemplate>
        </ItemsControl>
    </WrapPanel>
</ScrollViewer>
```

### 背景图片

```xml
<Grid>
    <!-- 背景层 -->
    <Image Source="assets/background.jpg" 
           Stretch="UniformToFill"
           Opacity="0.5"/>
    
    <!-- 内容层 -->
    <StackPanel VerticalAlignment="Center" HorizontalAlignment="Center">
        <TextBlock Text="标题" FontSize="48" Foreground="White"/>
        <Button Content="开始"/>
    </StackPanel>
</Grid>
```

### 缩略图加载

```xml
<Grid Columns="Auto,*" Spacing="12">
    <Border Grid.Column="0" Width="120" Height="90" CornerRadius="4">
        <Image Source="{Binding ThumbnailPath}"
               Stretch="UniformToFill"
               DecodeWidth="240"
               IsAsync="true"/>
    </Border>
    
    <StackPanel Grid.Column="1" Orientation="Vertical" Spacing="4">
        <TextBlock Text="{Binding Title}" FontWeight="Bold"/>
        <TextBlock Text="{Binding Description}" 
                   Foreground="Gray"
                   TextTrimming="CharacterEllipsis"/>
    </StackPanel>
</Grid>
```

### 图标按钮

```xml
<Button Width="40" Height="40" Style="IconButton">
    <Image Source="assets/icons/add.png" Width="24" Height="24"/>
</Button>

<StackPanel Orientation="Horizontal" Spacing="8">
    <Button Style="ToolButton">
        <Image Source="assets/icons/cut.png" Width="16" Height="16"/>
    </Button>
    <Button Style="ToolButton">
        <Image Source="assets/icons/copy.png" Width="16" Height="16"/>
    </Button>
    <Button Style="ToolButton">
        <Image Source="assets/icons/paste.png" Width="16" Height="16"/>
    </Button>
</StackPanel>
```

### 图片预览

```xml
<Border Background="#1A1A1A">
    <ScrollViewer HorizontalScrollBarVisibility="Auto"
                  VerticalScrollBarVisibility="Auto">
        <Image Source="{Binding PreviewImage}"
               Stretch="None"
               RenderTransform="{Binding ZoomTransform}"/>
    </ScrollViewer>
</Border>
```

### 带状态指示的图片

```xml
<Grid Width="300" Height="200">
    <!-- 加载中 -->
    <ProgressBar IsIndeterminate="true"
                 IsVisible="{Binding IsLoading}"
                 VerticalAlignment="Center"
                 HorizontalAlignment="Center"
                 Width="100"/>
    
    <!-- 加载失败 -->
    <StackPanel IsVisible="{Binding IsError}"
                VerticalAlignment="Center"
                HorizontalAlignment="Center">
        <TextBlock Text="⚠️" FontSize="32" HorizontalAlignment="Center"/>
        <TextBlock Text="加载失败" Foreground="Gray"/>
        <Button Content="重试" Click="ReloadCommand"/>
    </StackPanel>
    
    <!-- 图片 -->
    <Image Source="{Binding ImageSource}"
           Stretch="Uniform"
           IsVisible="{Binding IsLoaded}"
           IsAsync="true"/>
</Grid>
```

## 图片格式支持

| 格式 | 说明 | 建议用途 |
|------|------|----------|
| PNG | 无损压缩，支持透明 | 图标、Logo、UI元素 |
| JPG/JPEG | 有损压缩，文件小 | 照片、背景图 |
| BMP | 无压缩，文件大 | 无需压缩的场景 |
| GIF | 支持动画 | 简单动画（如支持） |
| ICO | 图标格式 | 应用程序图标 |
| WEBP | 高压缩率 | 现代浏览器/应用 |

## 异步加载

```cpp
// 异步加载大图片
image->SetSourceAsync(L"assets/hd_photo.jpg", 
    [](bool success, const std::wstring& error) {
        if (success) {
            // 加载成功
        } else {
            // 加载失败，显示错误
            std::wcerr << L"加载失败: " << error << std::endl;
        }
    });

// 带进度回调
image->SetSourceAsync(L"assets/large_file.jpg",
    [](int progress) {
        // 更新进度条
        progressBar->SetValue(progress);
    },
    [](bool success) {
        // 完成回调
    });
```

## 解码尺寸优化

```xml
<!-- 显示缩略图时限制解码尺寸，节省内存 -->
<Image Source="large_photo.jpg"
       DecodeWidth="400"
       DecodeHeight="300"
       Stretch="Uniform"/>
```

```cpp
// 代码中设置解码尺寸
image->SetDecodeSize(400, 300);
image->SetSource(L"large_photo.jpg");
```

## 数据绑定

### 绑定图片路径

```xml
<!-- 本地路径 -->
<Image Source="{Binding LocalImagePath}"/>

<!-- 网络路径（自动下载缓存） -->
<Image Source="{Binding RemoteImageUrl}" IsAsync="true"/>

<!-- 相对路径（相对于应用目录） -->
<Image Source="assets/images/{Binding ImageName}"/>
```

### 绑定字节数组

```cpp
// 从数据库/网络加载图片数据
std::vector<uint8_t> imageData = LoadImageFromDatabase(id);
image->SetSourceFromBytes(imageData);
```

## 图片缓存

```cpp
// 启用图片缓存
Image::SetCacheEnabled(true);

// 设置缓存大小限制（MB）
Image::SetCacheSizeLimit(100);

// 清除缓存
Image::ClearCache();

// 预加载图片到缓存
Image::PreloadToCache(L"assets/important.png");
```

## 样式定制

### 圆角图片

```xml
<Border CornerRadius="8" ClipToBounds="true">
    <Image Source="photo.jpg" Stretch="UniformToFill"/>
</Border>
```

### 带边框的图片

```xml
<Border BorderThickness="2" BorderColor="White" CornerRadius="4">
    <Image Source="avatar.jpg" Width="100" Height="100"/>
</Border>
```

### 灰度效果

```xml
<Image Source="photo.jpg">
    <Image.Effect>
        <GrayscaleEffect/>
    </Image.Effect>
</Image>
```

## 最佳实践

### 1. 选择合适的缩放模式

```xml
<!-- 照片展示：Uniform 保持比例 -->
<Image Source="photo.jpg" Stretch="Uniform"/>

<!-- 背景图：UniformToFill 填满裁剪 -->
<Image Source="bg.jpg" Stretch="UniformToFill"/>

<!-- 图标：None 或 Uniform -->
<Image Source="icon.png" Stretch="None" Width="32" Height="32"/>
```

### 2. 大图片异步加载

```xml
<!-- 推荐：大图片使用异步加载 -->
<Image Source="hd_image.jpg" IsAsync="true"/>

<!-- 不推荐：同步加载会阻塞 UI -->
<Image Source="hd_image.jpg"/>
```

### 3. 限制解码尺寸

```xml
<!-- 显示缩略图时限制尺寸，节省内存 -->
<Image Source="large.jpg" DecodeWidth="400"/>
```

### 4. 处理加载失败

```xml
<Grid>
    <Image Source="{Binding ImagePath}" FallbackSource="assets/default.png"/>
</Grid>
```

### 5. 使用占位符

```xml
<Grid>
    <Border Background="#F0F0F0" IsVisible="{Binding IsLoading}"/>
    <Image Source="{Binding ImageUrl}" IsAsync="true"/>
</Grid>
```

## 性能优化

1. **异步加载**：大图片使用 `IsAsync="true"`
2. **解码尺寸**：使用 `DecodeWidth/DecodeHeight` 限制内存
3. **图片缓存**：启用缓存避免重复加载
4. **延迟加载**：不在可视区域的图片延迟加载
5. **格式选择**：根据场景选择合适的图片格式

## 参见

- [Border](Border.md) - 边框容器
- [数据绑定](../architecture/MVVM_DataBinding_Design.md)
