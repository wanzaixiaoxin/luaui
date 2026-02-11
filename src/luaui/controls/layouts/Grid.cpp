#include "layouts/Grid.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

// 静态成员定义
std::unordered_map<ControlID, Grid::CellInfo> Grid::s_cellInfo;

Grid::Grid() {}

void Grid::AddColumn(const GridLength& width) {
    m_columns.push_back(width);
    m_columnWidths.push_back(0);
}

void Grid::AddRow(const GridLength& height) {
    m_rows.push_back(height);
    m_rowHeights.push_back(0);
}

void Grid::SetColumn(const std::shared_ptr<IControl>& control, int column) {
    if (!control) return;
    s_cellInfo[control->GetID()].column = column;
}

void Grid::SetRow(const std::shared_ptr<IControl>& control, int row) {
    if (!control) return;
    s_cellInfo[control->GetID()].row = row;
}

void Grid::SetColumnSpan(const std::shared_ptr<IControl>& control, int span) {
    if (!control) return;
    s_cellInfo[control->GetID()].columnSpan = span;
}

void Grid::SetRowSpan(const std::shared_ptr<IControl>& control, int span) {
    if (!control) return;
    s_cellInfo[control->GetID()].rowSpan = span;
}

int Grid::GetColumn(const std::shared_ptr<IControl>& control) {
    if (!control) return 0;
    auto it = s_cellInfo.find(control->GetID());
    return (it != s_cellInfo.end()) ? it->second.column : 0;
}

int Grid::GetRow(const std::shared_ptr<IControl>& control) {
    if (!control) return 0;
    auto it = s_cellInfo.find(control->GetID());
    return (it != s_cellInfo.end()) ? it->second.row : 0;
}

int Grid::GetColumnSpan(const std::shared_ptr<IControl>& control) {
    if (!control) return 1;
    auto it = s_cellInfo.find(control->GetID());
    return (it != s_cellInfo.end()) ? it->second.columnSpan : 1;
}

int Grid::GetRowSpan(const std::shared_ptr<IControl>& control) {
    if (!control) return 1;
    auto it = s_cellInfo.find(control->GetID());
    return (it != s_cellInfo.end()) ? it->second.rowSpan : 1;
}

rendering::Size Grid::OnMeasureChildren(const rendering::Size& availableSize) {
    // 简化实现：如果没有行列定义，则测量所有子控件
    if (m_columns.empty() && m_rows.empty()) {
        return Panel::OnMeasureChildren(availableSize);
    }
    
    // 先测量所有子控件
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            layoutable->Measure(constraint);
        }
    }
    
    // 计算网格尺寸
    CalculateSizes(availableSize);
    
    float totalWidth = 0;
    for (float w : m_columnWidths) totalWidth += w;
    
    float totalHeight = 0;
    for (float h : m_rowHeights) totalHeight += h;
    
    return rendering::Size(totalWidth, totalHeight);
}

rendering::Size Grid::OnArrangeChildren(const rendering::Size& finalSize) {
    auto* render = GetRender();
    if (!render) return finalSize;
    
    auto contentRect = render->GetRenderRect();
    
    // 计算每列和每行的起始位置
    std::vector<float> colStarts(m_columnWidths.size(), 0);
    std::vector<float> rowStarts(m_rowHeights.size(), 0);
    
    float x = contentRect.x;
    for (size_t i = 0; i < m_columnWidths.size(); ++i) {
        colStarts[i] = x;
        x += m_columnWidths[i];
    }
    
    float y = contentRect.y;
    for (size_t i = 0; i < m_rowHeights.size(); ++i) {
        rowStarts[i] = y;
        y += m_rowHeights[i];
    }
    
    // 排列子控件
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        if (auto* layoutable = child->AsLayoutable()) {
            int col = GetColumn(child);
            int row = GetRow(child);
            int colSpan = GetColumnSpan(child);
            int rowSpan = GetRowSpan(child);
            
            // 计算子控件的位置和大小
            float childX = (col < (int)colStarts.size()) ? colStarts[col] : contentRect.x;
            float childY = (row < (int)rowStarts.size()) ? rowStarts[row] : contentRect.y;
            
            float childWidth = 0;
            float childHeight = 0;
            
            for (int c = col; c < col + colSpan && c < (int)m_columnWidths.size(); ++c) {
                childWidth += m_columnWidths[c];
            }
            
            for (int r = row; r < row + rowSpan && r < (int)m_rowHeights.size(); ++r) {
                childHeight += m_rowHeights[r];
            }
            
            layoutable->Arrange(rendering::Rect(childX, childY, childWidth, childHeight));
        }
    }
    
    return finalSize;
}

void Grid::CalculateSizes(const rendering::Size& availableSize) {
    // 简化实现：计算行列大小
    // Auto - 根据内容
    // Pixel - 固定像素
    // Star - 按比例分配剩余空间
    
    float totalStarWidth = 0;
    float totalStarHeight = 0;
    float fixedWidth = 0;
    float fixedHeight = 0;
    
    // 第一遍：计算固定大小和 Star 权重
    for (size_t i = 0; i < m_columns.size(); ++i) {
        if (m_columns[i].IsPixel()) {
            m_columnWidths[i] = m_columns[i].value;
            fixedWidth += m_columnWidths[i];
        } else if (m_columns[i].IsStar()) {
            totalStarWidth += m_columns[i].value;
        }
    }
    
    for (size_t i = 0; i < m_rows.size(); ++i) {
        if (m_rows[i].IsPixel()) {
            m_rowHeights[i] = m_rows[i].value;
            fixedHeight += m_rowHeights[i];
        } else if (m_rows[i].IsStar()) {
            totalStarHeight += m_rows[i].value;
        }
    }
    
    // 第二遍：计算 Star 大小
    float remainingWidth = std::max(0.0f, availableSize.width - fixedWidth);
    float remainingHeight = std::max(0.0f, availableSize.height - fixedHeight);
    
    for (size_t i = 0; i < m_columns.size(); ++i) {
        if (m_columns[i].IsStar()) {
            m_columnWidths[i] = (m_columns[i].value / totalStarWidth) * remainingWidth;
        }
    }
    
    for (size_t i = 0; i < m_rows.size(); ++i) {
        if (m_rows[i].IsStar()) {
            m_rowHeights[i] = (m_rows[i].value / totalStarHeight) * remainingHeight;
        }
    }
}

} // namespace controls
} // namespace luaui
