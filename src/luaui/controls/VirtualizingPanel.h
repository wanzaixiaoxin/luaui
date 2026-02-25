#pragma once

#include "Panel.h"
#include "layouts/ScrollViewer.h"
#include "../rendering/Types.h"
#include <functional>
#include <memory>
#include <any>
#include <vector>

namespace luaui {
namespace controls {

// ============================================================================
// VirtualizationMode - 虚拟化模式
// ============================================================================
enum class VirtualizationMode {
    Standard,       // 标准虚拟化 - 只创建可见项
    Recycling,      // 回收模式 - 重用容器（性能更好）
    None            // 不虚拟化 - 创建所有项
};

// ============================================================================
// IVirtualizingContainer - 虚拟化容器接口
// 用于重用容器项
// ============================================================================
class IVirtualizingContainer {
public:
    virtual ~IVirtualizingContainer() = default;
    
    // 准备重用（清空数据）
    virtual void PrepareForReuse() = 0;
    
    // 绑定数据
    virtual void BindData(int index, const std::any& data) = 0;
    
    // 获取容器高度（固定高度模式）
    virtual float GetContainerHeight() const = 0;
};

// ============================================================================
// VirtualizingPanel - 虚拟化面板
// 只创建和渲染可见区域内的子项
// ============================================================================
class VirtualizingPanel : public Panel {
public:
    VirtualizingPanel();
    virtual ~VirtualizingPanel();
    
    std::string GetTypeName() const override { return "VirtualizingPanel"; }
    
    // 虚拟化模式
    VirtualizationMode GetVirtualizationMode() const { return m_virtualizationMode; }
    void SetVirtualizationMode(VirtualizationMode mode) { m_virtualizationMode = mode; }
    
    // 固定项高度（-1 表示自动计算，性能较低）
    float GetItemHeight() const { return m_itemHeight; }
    void SetItemHeight(float height) { m_itemHeight = height; if (auto* layout = GetLayout()) layout->InvalidateMeasure(); }
    
    // 缓存项数量（可见区域上下额外缓存的项数）
    int GetCacheLength() const { return m_cacheLength; }
    void SetCacheLength(int count) { m_cacheLength = count; }
    
    // 数据项数量（虚拟化数据源）
    int GetItemCount() const { return m_itemCount; }
    void SetItemCount(int count);
    
    // 设置数据获取回调
    using DataCallback = std::function<std::any(int index)>;
    void SetDataCallback(DataCallback callback) { m_dataCallback = callback; }
    
    // 设置容器创建回调
    using ContainerFactory = std::function<std::shared_ptr<Control>()>;
    void SetContainerFactory(ContainerFactory factory) { m_containerFactory = factory; }
    
    // 获取可见区域范围
    void GetVisibleRange(int& firstVisibleIndex, int& lastVisibleIndex) const;
    
    // 滚动到指定项
    void ScrollIntoView(int index);
    
    // 获取总高度（用于滚动条）
    float GetTotalHeight() const;

protected:
    void InitializeComponents() override;
    
    // 布局覆盖
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    
    // 渲染覆盖 - 只渲染可见项
    void OnRenderChildren(rendering::IRenderContext* context) override;
    
    // 滚动处理
    void OnScrollChanged(float newOffset);
    
private:
    // 内部结构：容器信息
    struct ContainerInfo {
        std::shared_ptr<Control> container;
        int itemIndex = -1;  // -1 表示未使用
        bool isInUse = false;
    };
    
    // 核心方法
    void UpdateVisibleContainers();
    void RecycleContainer(std::shared_ptr<Control> container);
    std::shared_ptr<Control> GetRecycledContainer();
    std::shared_ptr<Control> CreateNewContainer();
    void PositionContainer(std::shared_ptr<Control> container, int index);
    
    // 计算可见范围
    void CalculateVisibleRange(float viewportTop, float viewportHeight,
                               int& firstIndex, int& lastIndex);
    
    // 测量项高度（如果固定高度未设置）
    float MeasureItemHeight();
    
    // 成员变量
    VirtualizationMode m_virtualizationMode = VirtualizationMode::Recycling;
    float m_itemHeight = -1.0f;  // -1 表示未设置，需要测量
    int m_cacheLength = 2;       // 上下各缓存 2 项
    int m_itemCount = 0;
    
    float m_scrollOffset = 0.0f;  // 当前滚动偏移
    float m_viewportHeight = 0.0f; // 视口高度
    
    // 容器管理
    std::vector<ContainerInfo> m_containers;      // 所有容器（包括使用和回收的）
    std::vector<std::shared_ptr<Control>> m_recycledContainers;
    
    // 回调
    DataCallback m_dataCallback;
    ContainerFactory m_containerFactory;
    
    // 缓存测量高度
    float m_cachedItemHeight = 0.0f;
    bool m_hasMeasuredItemHeight = false;
    
    // 滚动条
    std::shared_ptr<ScrollViewer> m_scrollViewer;
};

} // namespace controls
} // namespace luaui
