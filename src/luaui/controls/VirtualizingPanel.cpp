#include "VirtualizingPanel.h"
#include "../rendering/IRenderContext.h"
#include "../utils/Logger.h"
#include "../core/Components/LayoutComponent.h"
#include "../core/Components/RenderComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

VirtualizingPanel::VirtualizingPanel() {
    utils::Logger::Debug("[VirtualizingPanel] Created");
}

VirtualizingPanel::~VirtualizingPanel() {
    utils::Logger::Debug("[VirtualizingPanel] Destroyed");
}

void VirtualizingPanel::SetItemCount(int count) {
    if (m_itemCount != count) {
        m_itemCount = count;
        // 清空现有容器，因为数据完全改变了
        m_containers.clear();
        m_recycledContainers.clear();
        if (auto* layout = GetLayout()) layout->InvalidateMeasure();
    }
}

void VirtualizingPanel::GetVisibleRange(int& firstVisibleIndex, int& lastVisibleIndex) const {
    firstVisibleIndex = -1;
    lastVisibleIndex = -1;
    
    for (const auto& info : m_containers) {
        if (info.isInUse && info.itemIndex >= 0) {
            if (firstVisibleIndex == -1 || info.itemIndex < firstVisibleIndex) {
                firstVisibleIndex = info.itemIndex;
            }
            if (lastVisibleIndex == -1 || info.itemIndex > lastVisibleIndex) {
                lastVisibleIndex = info.itemIndex;
            }
        }
    }
}

void VirtualizingPanel::ScrollIntoView(int index) {
    if (index < 0 || index >= m_itemCount) return;
    
    float itemHeight = GetItemHeight() > 0 ? GetItemHeight() : MeasureItemHeight();
    if (itemHeight <= 0) return;
    
    float targetOffset = index * itemHeight;
    
    // 确保目标项在可见区域内
    if (targetOffset < m_scrollOffset) {
        m_scrollOffset = targetOffset;
    } else if (targetOffset + itemHeight > m_scrollOffset + m_viewportHeight) {
        m_scrollOffset = targetOffset + itemHeight - m_viewportHeight;
    }
    
    // 限制滚动范围
    float maxOffset = std::max(0.0f, GetTotalHeight() - m_viewportHeight);
    m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxOffset);
    
    UpdateVisibleContainers();
    if (auto* layout = GetLayout()) layout->InvalidateMeasure();
}

float VirtualizingPanel::GetTotalHeight() const {
    if (m_itemCount <= 0) return 0.0f;
    
    float itemHeight = GetItemHeight() > 0 ? GetItemHeight() : 
                      (m_hasMeasuredItemHeight ? m_cachedItemHeight : 24.0f);
    return m_itemCount * itemHeight;
}

void VirtualizingPanel::InitializeComponents() {
    Panel::InitializeComponents();
    
    // 创建内部 ScrollViewer
    // 简化实现：VirtualizingPanel 本身处理滚动逻辑
}

rendering::Size VirtualizingPanel::OnMeasure(const rendering::Size& availableSize) {
    // 测量虚拟化面板
    float totalHeight = GetTotalHeight();
    
    // 如果有固定项高度，可以精确计算
    if (m_itemHeight > 0) {
        return rendering::Size(availableSize.width, std::min(totalHeight, availableSize.height));
    }
    
    // 否则返回一个估计值
    return rendering::Size(availableSize.width, availableSize.height);
}

rendering::Size VirtualizingPanel::OnArrangeChildren(const rendering::Size& finalSize) {
    m_viewportHeight = finalSize.height;
    
    // 更新可见容器
    UpdateVisibleContainers();
    
    // 安排可见容器的位置
    float itemHeight = GetItemHeight() > 0 ? GetItemHeight() : MeasureItemHeight();
    
    for (auto& info : m_containers) {
        if (info.isInUse && info.container && info.itemIndex >= 0) {
            float y = info.itemIndex * itemHeight - m_scrollOffset;
            auto* layout = info.container->GetLayout();
            if (layout) {
                layout->SetWidth(finalSize.width);
                layout->SetHeight(itemHeight);
                layout->Arrange(rendering::Rect(0, y, finalSize.width, itemHeight));
            }
        }
    }
    
    return finalSize;
}

void VirtualizingPanel::OnRenderChildren(rendering::IRenderContext* context) {
    // 只渲染可见的容器
    for (auto& info : m_containers) {
        if (info.isInUse && info.container && info.itemIndex >= 0) {
            // 检查是否在视口内
            auto* render = info.container->GetRender();
            if (render) {
                auto bounds = render->GetRenderRect();
                if (bounds.y + bounds.height > 0 && bounds.y < m_viewportHeight) {
                    render->Render(context);
                }
            }
        }
    }
}

void VirtualizingPanel::UpdateVisibleContainers() {
    if (m_itemCount <= 0) {
        // 回收所有容器
        for (auto& info : m_containers) {
            if (info.isInUse) {
                RecycleContainer(info.container);
                info.isInUse = false;
                info.itemIndex = -1;
            }
        }
        return;
    }
    
    // 计算新的可见范围
    int firstVisible, lastVisible;
    CalculateVisibleRange(m_scrollOffset, m_viewportHeight, firstVisible, lastVisible);
    
    if (firstVisible < 0 || lastVisible < 0) return;
    
    // 扩展缓存范围
    firstVisible = std::max(0, firstVisible - m_cacheLength);
    lastVisible = std::min(m_itemCount - 1, lastVisible + m_cacheLength);
    
    // 标记所有现有容器为待回收
    std::vector<bool> keepContainer(m_containers.size(), false);
    
    // 第一步：重用已存在的容器
    for (int index = firstVisible; index <= lastVisible; ++index) {
        bool found = false;
        
        // 查找是否已经有这个索引的容器
        for (size_t i = 0; i < m_containers.size(); ++i) {
            if (m_containers[i].isInUse && m_containers[i].itemIndex == index) {
                keepContainer[i] = true;
                found = true;
                break;
            }
        }
        
        if (!found) {
            // 需要创建或重用容器
            std::shared_ptr<Control> container;
            
            // 尝试从回收池获取
            container = GetRecycledContainer();
            
            // 如果没有回收的容器，创建新的
            if (!container && m_containerFactory) {
                container = m_containerFactory();
                if (container) {
                    ContainerInfo info;
                    info.container = container;
                    m_containers.push_back(info);
                    keepContainer.push_back(true);
                }
            }
            
            // 如果获取到容器，绑定数据并设置位置
            if (container) {
                // 找到对应的 ContainerInfo
                for (auto& info : m_containers) {
                    if (info.container == container && !info.isInUse) {
                        info.isInUse = true;
                        info.itemIndex = index;
                        PositionContainer(container, index);
                        break;
                    }
                }
            }
        }
    }
    
    // 第二步：回收不再使用的容器
    for (size_t i = 0; i < m_containers.size(); ++i) {
        if (m_containers[i].isInUse && !keepContainer[i]) {
            RecycleContainer(m_containers[i].container);
            m_containers[i].isInUse = false;
            m_containers[i].itemIndex = -1;
        }
    }
}

void VirtualizingPanel::RecycleContainer(std::shared_ptr<Control> container) {
    if (!container) return;
    
    // 如果实现了 IVirtualizingContainer 接口，调用 PrepareForReuse
    // 简化实现：只是隐藏容器
    container->SetIsVisible(false);
    
    m_recycledContainers.push_back(container);
}

std::shared_ptr<Control> VirtualizingPanel::GetRecycledContainer() {
    if (m_recycledContainers.empty()) return nullptr;
    
    auto container = m_recycledContainers.back();
    m_recycledContainers.pop_back();
    container->SetIsVisible(true);
    
    return container;
}

std::shared_ptr<Control> VirtualizingPanel::CreateNewContainer() {
    if (!m_containerFactory) return nullptr;
    return m_containerFactory();
}

void VirtualizingPanel::PositionContainer(std::shared_ptr<Control> container, int index) {
    if (!container) return;
    
    float itemHeight = GetItemHeight() > 0 ? GetItemHeight() : MeasureItemHeight();
    (void)(index * itemHeight - m_scrollOffset);  // y position, unused for now
    
    auto* layout = container->GetLayout();
    if (layout) {
        layout->SetWidth(GetLayout() ? GetLayout()->GetWidth() : 100.0f);
        layout->SetHeight(itemHeight);
    }
    
    // 绑定数据
    if (m_dataCallback) {
        auto data = m_dataCallback(index);
        // 这里需要通过某种方式将数据传递给容器
        // 简化：假设容器有特定的数据绑定方式
    }
}

void VirtualizingPanel::CalculateVisibleRange(float viewportTop, float viewportHeight,
                                               int& firstIndex, int& lastIndex) {
    firstIndex = -1;
    lastIndex = -1;
    
    if (m_itemCount <= 0) return;
    
    float itemHeight = GetItemHeight() > 0 ? GetItemHeight() : MeasureItemHeight();
    if (itemHeight <= 0) return;
    
    firstIndex = static_cast<int>(viewportTop / itemHeight);
    lastIndex = static_cast<int>((viewportTop + viewportHeight) / itemHeight);
    
    firstIndex = std::max(0, firstIndex);
    lastIndex = std::min(m_itemCount - 1, lastIndex);
}

float VirtualizingPanel::MeasureItemHeight() {
    if (m_hasMeasuredItemHeight && m_cachedItemHeight > 0) {
        return m_cachedItemHeight;
    }
    
    if (!m_containerFactory) return 24.0f;  // 默认高度
    
    // 创建一个临时容器来测量高度
    auto tempContainer = m_containerFactory();
    if (!tempContainer) return 24.0f;
    
    // 绑定示例数据
    if (m_dataCallback) {
        auto data = m_dataCallback(0);
        // 数据绑定...
    }
    
    // 测量
    auto* layout = tempContainer->GetLayout();
    if (layout) {
        interfaces::LayoutConstraint constraint;
        constraint.available = rendering::Size(GetLayout() ? GetLayout()->GetWidth() : 100.0f, 1000.0f);
        auto desiredSize = layout->Measure(constraint);
        m_cachedItemHeight = desiredSize.height;
    }
    if (m_cachedItemHeight <= 0) m_cachedItemHeight = 24.0f;
    
    m_hasMeasuredItemHeight = true;
    
    return m_cachedItemHeight;
}

} // namespace controls
} // namespace luaui
