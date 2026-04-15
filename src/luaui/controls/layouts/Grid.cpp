#include "layouts/Grid.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include <algorithm>

namespace luaui {
namespace controls {

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
    m_cellInfo[control->GetID()].column = column;
}

void Grid::SetRow(const std::shared_ptr<IControl>& control, int row) {
    if (!control) return;
    m_cellInfo[control->GetID()].row = row;
}

void Grid::SetColumnSpan(const std::shared_ptr<IControl>& control, int span) {
    if (!control) return;
    m_cellInfo[control->GetID()].columnSpan = span;
}

void Grid::SetRowSpan(const std::shared_ptr<IControl>& control, int span) {
    if (!control) return;
    m_cellInfo[control->GetID()].rowSpan = span;
}

int Grid::GetColumn(const std::shared_ptr<IControl>& control) const {
    if (!control) return 0;
    auto it = m_cellInfo.find(control->GetID());
    return (it != m_cellInfo.end()) ? it->second.column : 0;
}

int Grid::GetRow(const std::shared_ptr<IControl>& control) const {
    if (!control) return 0;
    auto it = m_cellInfo.find(control->GetID());
    return (it != m_cellInfo.end()) ? it->second.row : 0;
}

int Grid::GetColumnSpan(const std::shared_ptr<IControl>& control) const {
    if (!control) return 1;
    auto it = m_cellInfo.find(control->GetID());
    return (it != m_cellInfo.end()) ? it->second.columnSpan : 1;
}

int Grid::GetRowSpan(const std::shared_ptr<IControl>& control) const {
    if (!control) return 1;
    auto it = m_cellInfo.find(control->GetID());
    return (it != m_cellInfo.end()) ? it->second.rowSpan : 1;
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
    // 完整实现：计算行列大小
    // Auto - 根据内容（子控件最大尺寸）
    // Pixel - 固定像素
    // Star - 按比例分配剩余空间
    
    // 初始化所有大小为0
    std::fill(m_columnWidths.begin(), m_columnWidths.end(), 0.0f);
    std::fill(m_rowHeights.begin(), m_rowHeights.end(), 0.0f);
    
    // ========== 第一步：计算Pixel固定大小 ==========
    float fixedWidth = 0;
    float fixedHeight = 0;
    
    for (size_t i = 0; i < m_columns.size(); ++i) {
        if (m_columns[i].IsPixel()) {
            m_columnWidths[i] = m_columns[i].value;
            fixedWidth += m_columnWidths[i];
        }
    }
    
    for (size_t i = 0; i < m_rows.size(); ++i) {
        if (m_rows[i].IsPixel()) {
            m_rowHeights[i] = m_rows[i].value;
            fixedHeight += m_rowHeights[i];
        }
    }
    
    // ========== 第二步：计算Auto大小 ==========
    // 对于每个Auto列，找到该列中所有子控件的最大宽度
    // 对于每个Auto行，找到该行中所有子控件的最大高度
    
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        int col = GetColumn(child);
        int row = GetRow(child);
        int colSpan = GetColumnSpan(child);
        int rowSpan = GetRowSpan(child);
        
        if (auto* layoutable = child->AsLayoutable()) {
            auto desired = layoutable->GetDesiredSize();
            
            // 处理Auto列
            // 如果子控件跨多列，且所有列都是Auto，则平均分配
            // 如果子控件跨多列，且部分是Auto，则只分配给Auto列
            if (colSpan == 1 && col >= 0 && col < (int)m_columns.size()) {
                if (m_columns[col].IsAuto()) {
                    m_columnWidths[col] = std::max(m_columnWidths[col], desired.width);
                }
            } else if (colSpan > 1) {
                // 跨列情况：计算Auto列的数量
                int autoColCount = 0;
                for (int c = col; c < col + colSpan && c < (int)m_columns.size(); ++c) {
                    if (m_columns[c].IsAuto()) autoColCount++;
                }
                if (autoColCount > 0) {
                    // 计算已分配的固定宽度
                    float allocatedWidth = 0;
                    for (int c = col; c < col + colSpan && c < (int)m_columns.size(); ++c) {
                        if (m_columns[c].IsPixel()) {
                            allocatedWidth += m_columnWidths[c];
                        }
                    }
                    // 剩余宽度平均分配给Auto列
                    float remainingWidth = std::max(0.0f, desired.width - allocatedWidth);
                    float widthPerAutoCol = remainingWidth / autoColCount;
                    for (int c = col; c < col + colSpan && c < (int)m_columns.size(); ++c) {
                        if (m_columns[c].IsAuto()) {
                            m_columnWidths[c] = std::max(m_columnWidths[c], widthPerAutoCol);
                        }
                    }
                }
            }
            
            // 处理Auto行（逻辑同列）
            if (rowSpan == 1 && row >= 0 && row < (int)m_rows.size()) {
                if (m_rows[row].IsAuto()) {
                    m_rowHeights[row] = std::max(m_rowHeights[row], desired.height);
                }
            } else if (rowSpan > 1) {
                int autoRowCount = 0;
                for (int r = row; r < row + rowSpan && r < (int)m_rows.size(); ++r) {
                    if (m_rows[r].IsAuto()) autoRowCount++;
                }
                if (autoRowCount > 0) {
                    float allocatedHeight = 0;
                    for (int r = row; r < row + rowSpan && r < (int)m_rows.size(); ++r) {
                        if (m_rows[r].IsPixel()) {
                            allocatedHeight += m_rowHeights[r];
                        }
                    }
                    float remainingHeight = std::max(0.0f, desired.height - allocatedHeight);
                    float heightPerAutoRow = remainingHeight / autoRowCount;
                    for (int r = row; r < row + rowSpan && r < (int)m_rows.size(); ++r) {
                        if (m_rows[r].IsAuto()) {
                            m_rowHeights[r] = std::max(m_rowHeights[r], heightPerAutoRow);
                        }
                    }
                }
            }
        }
    }
    
    // 累加Auto大小
    float autoWidth = 0;
    float autoHeight = 0;
    for (size_t i = 0; i < m_columns.size(); ++i) {
        if (m_columns[i].IsAuto()) {
            autoWidth += m_columnWidths[i];
        }
    }
    for (size_t i = 0; i < m_rows.size(); ++i) {
        if (m_rows[i].IsAuto()) {
            autoHeight += m_rowHeights[i];
        }
    }
    
    // ========== 第三步：计算Star大小 ==========
    float totalStarWidth = 0;
    float totalStarHeight = 0;
    
    for (size_t i = 0; i < m_columns.size(); ++i) {
        if (m_columns[i].IsStar()) {
            totalStarWidth += m_columns[i].value;
        }
    }
    for (size_t i = 0; i < m_rows.size(); ++i) {
        if (m_rows[i].IsStar()) {
            totalStarHeight += m_rows[i].value;
        }
    }
    
    // 剩余空间 = 可用空间 - 固定大小 - Auto大小
    float remainingWidth = std::max(0.0f, availableSize.width - fixedWidth - autoWidth);
    float remainingHeight = std::max(0.0f, availableSize.height - fixedHeight - autoHeight);
    
    // 分配Star大小
    if (totalStarWidth > 0) {
        for (size_t i = 0; i < m_columns.size(); ++i) {
            if (m_columns[i].IsStar()) {
                m_columnWidths[i] = (m_columns[i].value / totalStarWidth) * remainingWidth;
            }
        }
    }
    
    if (totalStarHeight > 0) {
        for (size_t i = 0; i < m_rows.size(); ++i) {
            if (m_rows[i].IsStar()) {
                m_rowHeights[i] = (m_rows[i].value / totalStarHeight) * remainingHeight;
            }
        }
    }
}

} // namespace controls
} // namespace luaui
