#include "Grid.h"
#include <algorithm>
#include <unordered_map>

namespace luaui {
namespace controls {

// Static storage for attached properties
static std::unordered_map<Control*, int> s_rowMap;
static std::unordered_map<Control*, int> s_colMap;
static std::unordered_map<Control*, int> s_rowSpanMap;
static std::unordered_map<Control*, int> s_colSpanMap;

void Grid::SetRow(Control* control, int row) {
    if (control) s_rowMap[control] = std::max(0, row);
}

void Grid::SetColumn(Control* control, int column) {
    if (control) s_colMap[control] = std::max(0, column);
}

void Grid::SetRowSpan(Control* control, int span) {
    if (control) s_rowSpanMap[control] = std::max(1, span);
}

void Grid::SetColumnSpan(Control* control, int span) {
    if (control) s_colSpanMap[control] = std::max(1, span);
}

int Grid::GetRow(Control* control) {
    if (!control) return 0;
    auto it = s_rowMap.find(control);
    return (it != s_rowMap.end()) ? it->second : 0;
}

int Grid::GetColumn(Control* control) {
    if (!control) return 0;
    auto it = s_colMap.find(control);
    return (it != s_colMap.end()) ? it->second : 0;
}

int Grid::GetRowSpan(Control* control) {
    if (!control) return 1;
    auto it = s_rowSpanMap.find(control);
    return (it != s_rowSpanMap.end()) ? it->second : 1;
}

int Grid::GetColumnSpan(Control* control) {
    if (!control) return 1;
    auto it = s_colSpanMap.find(control);
    return (it != s_colSpanMap.end()) ? it->second : 1;
}

void Grid::AddRow(const GridLength& height, float minHeight, float maxHeight) {
    RowDefinition row;
    row.Height = height;
    row.MinHeight = minHeight;
    row.MaxHeight = maxHeight;
    m_rows.push_back(row);
}

void Grid::AddColumn(const GridLength& width, float minWidth, float maxWidth) {
    ColumnDefinition col;
    col.Width = width;
    col.MinWidth = minWidth;
    col.MaxWidth = maxWidth;
    m_columns.push_back(col);
}

void Grid::CalculateRowHeights(float availableHeight) {
    if (m_rows.empty()) {
        RowDefinition row;
        row.Height = GridLength::Auto();
        m_rows.push_back(row);
    }
    
    float totalStar = 0;
    float fixedAndAutoHeight = 0;
    
    // First pass: calculate pixel heights
    for (auto& row : m_rows) {
        switch (row.Height.Type) {
            case GridUnitType::Pixel:
                row.ActualHeight = std::max(row.MinHeight, 
                    std::min(row.Height.Value, row.MaxHeight));
                fixedAndAutoHeight += row.ActualHeight;
                break;
            case GridUnitType::Star:
                totalStar += row.Height.Value;
                break;
            case GridUnitType::Auto:
                row.ActualHeight = 0;
                break;
        }
    }
    
    // Calculate auto heights from children
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        int row = std::min(GetRow(child.get()), (int)m_rows.size() - 1);
        int rowSpan = std::min(GetRowSpan(child.get()), (int)m_rows.size() - row);
        
        if (rowSpan == 1 && m_rows[row].Height.Type == GridUnitType::Auto) {
            m_rows[row].ActualHeight = std::max(m_rows[row].ActualHeight, 
                child->GetDesiredSize().height);
        }
    }
    
    // Add auto heights
    for (auto& row : m_rows) {
        if (row.Height.Type == GridUnitType::Auto) {
            row.ActualHeight = std::max(row.MinHeight, 
                std::min(row.ActualHeight, row.MaxHeight));
            fixedAndAutoHeight += row.ActualHeight;
        }
    }
    
    // Calculate star heights
    if (totalStar > 0) {
        float remainingHeight = std::max(0.0f, availableHeight - fixedAndAutoHeight);
        float starValue = remainingHeight / totalStar;
        
        for (auto& row : m_rows) {
            if (row.Height.Type == GridUnitType::Star) {
                row.ActualHeight = std::max(row.MinHeight, 
                    std::min(starValue * row.Height.Value, row.MaxHeight));
            }
        }
    }
}

void Grid::CalculateColumnWidths(float availableWidth) {
    if (m_columns.empty()) {
        ColumnDefinition col;
        col.Width = GridLength::Auto();
        m_columns.push_back(col);
    }
    
    float totalStar = 0;
    float fixedAndAutoWidth = 0;
    
    // First pass: calculate pixel widths
    for (auto& col : m_columns) {
        switch (col.Width.Type) {
            case GridUnitType::Pixel:
                col.ActualWidth = std::max(col.MinWidth, 
                    std::min(col.Width.Value, col.MaxWidth));
                fixedAndAutoWidth += col.ActualWidth;
                break;
            case GridUnitType::Star:
                totalStar += col.Width.Value;
                break;
            case GridUnitType::Auto:
                col.ActualWidth = 0;
                break;
        }
    }
    
    // Calculate auto widths from children
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        int column = std::min(GetColumn(child.get()), (int)m_columns.size() - 1);
        int colSpan = std::min(GetColumnSpan(child.get()), (int)m_columns.size() - column);
        
        if (colSpan == 1 && m_columns[column].Width.Type == GridUnitType::Auto) {
            m_columns[column].ActualWidth = std::max(m_columns[column].ActualWidth, 
                child->GetDesiredSize().width);
        }
    }
    
    // Add auto widths
    for (auto& col : m_columns) {
        if (col.Width.Type == GridUnitType::Auto) {
            col.ActualWidth = std::max(col.MinWidth, 
                std::min(col.ActualWidth, col.MaxWidth));
            fixedAndAutoWidth += col.ActualWidth;
        }
    }
    
    // Calculate star widths
    if (totalStar > 0) {
        float remainingWidth = std::max(0.0f, availableWidth - fixedAndAutoWidth);
        float starValue = remainingWidth / totalStar;
        
        for (auto& col : m_columns) {
            if (col.Width.Type == GridUnitType::Star) {
                col.ActualWidth = std::max(col.MinWidth, 
                    std::min(starValue * col.Width.Value, col.MaxWidth));
            }
        }
    }
}

Size Grid::MeasureOverride(const Size& availableSize) {
    if (m_rows.empty()) AddRow();
    if (m_columns.empty()) AddColumn();
    
    // Measure all children
    for (auto& child : m_children) {
        if (child->GetIsVisible()) {
            child->Measure(availableSize);
        }
    }
    
    CalculateRowHeights(availableSize.height);
    CalculateColumnWidths(availableSize.width);
    
    Size desired;
    for (auto& row : m_rows) desired.height += row.ActualHeight;
    for (auto& col : m_columns) desired.width += col.ActualWidth;
    
    return desired;
}

Size Grid::ArrangeOverride(const Size& finalSize) {
    CalculateRowHeights(finalSize.height);
    CalculateColumnWidths(finalSize.width);
    
    // Get our position from render rect
    float baseX = m_renderRect.x;
    float baseY = m_renderRect.y;
    
    // Calculate positions
    std::vector<float> rowTops(m_rows.size());
    std::vector<float> colLefts(m_columns.size());
    
    float y = 0;
    for (size_t i = 0; i < m_rows.size(); ++i) {
        rowTops[i] = y;
        y += m_rows[i].ActualHeight;
    }
    
    float x = 0;
    for (size_t i = 0; i < m_columns.size(); ++i) {
        colLefts[i] = x;
        x += m_columns[i].ActualWidth;
    }
    
    // Arrange children with absolute coordinates
    for (auto& child : m_children) {
        if (!child->GetIsVisible()) continue;
        
        int row = std::min(GetRow(child.get()), (int)m_rows.size() - 1);
        int col = std::min(GetColumn(child.get()), (int)m_columns.size() - 1);
        int rowSpan = std::min(GetRowSpan(child.get()), (int)m_rows.size() - row);
        int colSpan = std::min(GetColumnSpan(child.get()), (int)m_columns.size() - col);
        
        float cellX = baseX + colLefts[col];
        float cellY = baseY + rowTops[row];
        float cellW = 0, cellH = 0;
        
        for (int i = 0; i < colSpan; ++i) cellW += m_columns[col + i].ActualWidth;
        for (int i = 0; i < rowSpan; ++i) cellH += m_rows[row + i].ActualHeight;
        
        child->Arrange(Rect(cellX, cellY, cellW, cellH));
    }
    
    return finalSize;
}

} // namespace controls
} // namespace luaui
