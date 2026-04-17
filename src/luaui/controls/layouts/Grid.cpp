#include "layouts/Grid.h"
#include "Interfaces/IControl.h"
#include "Interfaces/ILayoutable.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Logger.h"
#include <algorithm>

namespace luaui {
namespace controls {

Grid::Grid() {}

void Grid::AddColumn(const GridLength& width) {
    m_columns.push_back(width);
    m_columnWidths.push_back(0.0f);
    InvalidateGridLayout();
}

void Grid::SetColumnDefinition(size_t index, const GridLength& width) {
    if (index >= m_columns.size()) return;
    m_columns[index] = width;
    InvalidateGridLayout();
}

void Grid::ClearColumns() {
    m_columns.clear();
    m_columnWidths.clear();
    InvalidateGridLayout();
}

void Grid::AddRow(const GridLength& height) {
    m_rows.push_back(height);
    m_rowHeights.push_back(0.0f);
    InvalidateGridLayout();
}

void Grid::SetRowDefinition(size_t index, const GridLength& height) {
    if (index >= m_rows.size()) return;
    m_rows[index] = height;
    InvalidateGridLayout();
}

void Grid::ClearRows() {
    m_rows.clear();
    m_rowHeights.clear();
    InvalidateGridLayout();
}

void Grid::SetColumn(const std::shared_ptr<interfaces::IControl>& control, int column) {
    if (!control) return;
    m_cellInfo[control->GetID()].column = column;
    InvalidateGridLayout();
}

void Grid::SetRow(const std::shared_ptr<interfaces::IControl>& control, int row) {
    if (!control) return;
    m_cellInfo[control->GetID()].row = row;
    InvalidateGridLayout();
}

void Grid::SetColumnSpan(const std::shared_ptr<interfaces::IControl>& control, int span) {
    if (!control) return;
    m_cellInfo[control->GetID()].columnSpan = span;
    InvalidateGridLayout();
}

void Grid::SetRowSpan(const std::shared_ptr<interfaces::IControl>& control, int span) {
    if (!control) return;
    m_cellInfo[control->GetID()].rowSpan = span;
    InvalidateGridLayout();
}

int Grid::GetColumn(const std::shared_ptr<interfaces::IControl>& control) const {
    if (!control) return 0;
    auto it = m_cellInfo.find(control->GetID());
    return it != m_cellInfo.end() ? it->second.column : 0;
}

int Grid::GetRow(const std::shared_ptr<interfaces::IControl>& control) const {
    if (!control) return 0;
    auto it = m_cellInfo.find(control->GetID());
    return it != m_cellInfo.end() ? it->second.row : 0;
}

int Grid::GetColumnSpan(const std::shared_ptr<interfaces::IControl>& control) const {
    if (!control) return 1;
    auto it = m_cellInfo.find(control->GetID());
    return it != m_cellInfo.end() ? it->second.columnSpan : 1;
}

int Grid::GetRowSpan(const std::shared_ptr<interfaces::IControl>& control) const {
    if (!control) return 1;
    auto it = m_cellInfo.find(control->GetID());
    return it != m_cellInfo.end() ? it->second.rowSpan : 1;
}

rendering::Size Grid::OnMeasureChildren(const rendering::Size& availableSize) {
    if (m_columns.empty() && m_rows.empty() && m_cellInfo.empty()) {
        return Panel::OnMeasureChildren(availableSize);
    }

    const size_t effectiveColumns = GetEffectiveColumnCount();
    const size_t effectiveRows = GetEffectiveRowCount();

    m_columnWidths.assign(effectiveColumns, 0.0f);
    m_rowHeights.assign(effectiveRows, 0.0f);

    for (const auto& child : m_children) {
        if (!child || !child->GetIsVisible()) continue;

        if (auto* layoutable = child->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            layoutable->Measure(constraint);
        }
    }

    CalculateSizes(availableSize);

    float totalWidth = 0.0f;
    for (float width : m_columnWidths) {
        totalWidth += width;
    }

    float totalHeight = 0.0f;
    for (float height : m_rowHeights) {
        totalHeight += height;
    }

    utils::Logger::InfoF("[Grid Measure] avail=%.1fx%.1f desired=%.1fx%.1f cols=%zu rows=%zu",
        availableSize.width, availableSize.height, totalWidth, totalHeight, m_columnWidths.size(), m_rowHeights.size());
    for (size_t i = 0; i < m_columnWidths.size(); ++i) {
        utils::Logger::InfoF("[Grid Measure] col[%zu]=%.1f", i, m_columnWidths[i]);
    }

    return rendering::Size(totalWidth, totalHeight);
}

rendering::Size Grid::OnArrangeChildren(const rendering::Size& finalSize) {
    if (m_columns.empty() && m_rows.empty() && m_cellInfo.empty()) {
        return Panel::OnArrangeChildren(finalSize);
    }

    // Recalculate sizes based on actual final size (may differ from measure available size)
    CalculateSizes(finalSize);

    // Use relative positions (0,0) - the rendering pipeline applies cumulative transforms,
    // so child positions must be relative to the parent, not absolute screen coordinates
    std::vector<float> colStarts(m_columnWidths.size(), 0.0f);
    std::vector<float> rowStarts(m_rowHeights.size(), 0.0f);

    float x = 0.0f;
    for (size_t i = 0; i < m_columnWidths.size(); ++i) {
        colStarts[i] = x;
        x += m_columnWidths[i];
    }

    float y = 0.0f;
    for (size_t i = 0; i < m_rowHeights.size(); ++i) {
        rowStarts[i] = y;
        y += m_rowHeights[i];
    }

    for (const auto& child : m_children) {
        if (!child || !child->GetIsVisible()) continue;

        auto* layoutable = child->AsLayoutable();
        if (!layoutable) continue;

        const int col = GetColumn(child);
        const int row = GetRow(child);
        const int colSpan = std::max(1, GetColumnSpan(child));
        const int rowSpan = std::max(1, GetRowSpan(child));

        const float childX = (col >= 0 && col < static_cast<int>(colStarts.size())) ? colStarts[col] : 0.0f;
        const float childY = (row >= 0 && row < static_cast<int>(rowStarts.size())) ? rowStarts[row] : 0.0f;

        float childWidth = 0.0f;
        for (int c = std::max(0, col); c < col + colSpan && c < static_cast<int>(m_columnWidths.size()); ++c) {
            childWidth += m_columnWidths[c];
        }

        float childHeight = 0.0f;
        for (int r = std::max(0, row); r < row + rowSpan && r < static_cast<int>(m_rowHeights.size()); ++r) {
            childHeight += m_rowHeights[r];
        }

        utils::Logger::InfoF("[Grid Arrange] child=%s col=%d row=%d arrange=%.1fx%.1f@%.1f,%.1f",
            child->GetTypeName().c_str(), col, row, childWidth, childHeight, childX, childY);
        layoutable->Arrange(rendering::Rect(childX, childY, childWidth, childHeight));
    }

    return finalSize;
}

void Grid::CalculateSizes(const rendering::Size& availableSize) {
    const size_t effectiveColumns = GetEffectiveColumnCount();
    const size_t effectiveRows = GetEffectiveRowCount();

    m_columnWidths.assign(effectiveColumns, 0.0f);
    m_rowHeights.assign(effectiveRows, 0.0f);

    if (effectiveColumns == 0 && effectiveRows == 0) {
        return;
    }

    float fixedWidth = 0.0f;
    float fixedHeight = 0.0f;

    for (size_t i = 0; i < effectiveColumns; ++i) {
        const GridLength definition = GetColumnDefinitionAt(i);
        if (definition.IsPixel()) {
            m_columnWidths[i] = definition.value;
            fixedWidth += definition.value;
        }
    }

    for (size_t i = 0; i < effectiveRows; ++i) {
        const GridLength definition = GetRowDefinitionAt(i);
        if (definition.IsPixel()) {
            m_rowHeights[i] = definition.value;
            fixedHeight += definition.value;
        }
    }

    for (const auto& child : m_children) {
        if (!child || !child->GetIsVisible()) continue;

        auto* layoutable = child->AsLayoutable();
        if (!layoutable) continue;

        const rendering::Size desired = layoutable->GetDesiredSize();
        const int col = GetColumn(child);
        const int row = GetRow(child);
        const int colSpan = std::max(1, GetColumnSpan(child));
        const int rowSpan = std::max(1, GetRowSpan(child));

        if (colSpan == 1 && col >= 0 && col < static_cast<int>(effectiveColumns)) {
            if (GetColumnDefinitionAt(static_cast<size_t>(col)).IsAuto()) {
                m_columnWidths[col] = std::max(m_columnWidths[col], desired.width);
            }
        } else if (colSpan > 1) {
            int autoColCount = 0;
            float allocatedWidth = 0.0f;
            for (int c = std::max(0, col); c < col + colSpan && c < static_cast<int>(effectiveColumns); ++c) {
                const GridLength definition = GetColumnDefinitionAt(static_cast<size_t>(c));
                if (definition.IsAuto()) {
                    ++autoColCount;
                } else if (definition.IsPixel()) {
                    allocatedWidth += m_columnWidths[c];
                }
            }
            if (autoColCount > 0) {
                const float remainingWidth = std::max(0.0f, desired.width - allocatedWidth);
                const float widthPerAutoCol = remainingWidth / autoColCount;
                for (int c = std::max(0, col); c < col + colSpan && c < static_cast<int>(effectiveColumns); ++c) {
                    if (GetColumnDefinitionAt(static_cast<size_t>(c)).IsAuto()) {
                        m_columnWidths[c] = std::max(m_columnWidths[c], widthPerAutoCol);
                    }
                }
            }
        }

        if (rowSpan == 1 && row >= 0 && row < static_cast<int>(effectiveRows)) {
            if (GetRowDefinitionAt(static_cast<size_t>(row)).IsAuto()) {
                m_rowHeights[row] = std::max(m_rowHeights[row], desired.height);
            }
        } else if (rowSpan > 1) {
            int autoRowCount = 0;
            float allocatedHeight = 0.0f;
            for (int r = std::max(0, row); r < row + rowSpan && r < static_cast<int>(effectiveRows); ++r) {
                const GridLength definition = GetRowDefinitionAt(static_cast<size_t>(r));
                if (definition.IsAuto()) {
                    ++autoRowCount;
                } else if (definition.IsPixel()) {
                    allocatedHeight += m_rowHeights[r];
                }
            }
            if (autoRowCount > 0) {
                const float remainingHeight = std::max(0.0f, desired.height - allocatedHeight);
                const float heightPerAutoRow = remainingHeight / autoRowCount;
                for (int r = std::max(0, row); r < row + rowSpan && r < static_cast<int>(effectiveRows); ++r) {
                    if (GetRowDefinitionAt(static_cast<size_t>(r)).IsAuto()) {
                        m_rowHeights[r] = std::max(m_rowHeights[r], heightPerAutoRow);
                    }
                }
            }
        }
    }

    float autoWidth = 0.0f;
    for (size_t i = 0; i < effectiveColumns; ++i) {
        if (GetColumnDefinitionAt(i).IsAuto()) {
            autoWidth += m_columnWidths[i];
        }
    }

    float autoHeight = 0.0f;
    for (size_t i = 0; i < effectiveRows; ++i) {
        if (GetRowDefinitionAt(i).IsAuto()) {
            autoHeight += m_rowHeights[i];
        }
    }

    float totalStarWidth = 0.0f;
    for (size_t i = 0; i < effectiveColumns; ++i) {
        const GridLength definition = GetColumnDefinitionAt(i);
        if (definition.IsStar()) {
            totalStarWidth += definition.value;
        }
    }

    float totalStarHeight = 0.0f;
    for (size_t i = 0; i < effectiveRows; ++i) {
        const GridLength definition = GetRowDefinitionAt(i);
        if (definition.IsStar()) {
            totalStarHeight += definition.value;
        }
    }

    const bool infiniteWidth = availableSize.width >= 99990.0f;
    const bool infiniteHeight = availableSize.height >= 99990.0f;

    if (totalStarWidth > 0.0f) {
        if (infiniteWidth) {
            // When available width is infinite, Star columns behave like Auto
            for (const auto& child : m_children) {
                if (!child || !child->GetIsVisible()) continue;
                auto* layoutable = child->AsLayoutable();
                if (!layoutable) continue;
                const rendering::Size desired = layoutable->GetDesiredSize();
                const int col = GetColumn(child);
                const int colSpan = std::max(1, GetColumnSpan(child));
                if (colSpan == 1 && col >= 0 && col < static_cast<int>(effectiveColumns)) {
                    if (GetColumnDefinitionAt(static_cast<size_t>(col)).IsStar()) {
                        m_columnWidths[col] = std::max(m_columnWidths[col], desired.width);
                    }
                } else if (colSpan > 1) {
                    int starColCount = 0;
                    float allocatedWidth = 0.0f;
                    for (int c = std::max(0, col); c < col + colSpan && c < static_cast<int>(effectiveColumns); ++c) {
                        const GridLength definition = GetColumnDefinitionAt(static_cast<size_t>(c));
                        if (definition.IsStar()) {
                            ++starColCount;
                        } else {
                            allocatedWidth += m_columnWidths[c];
                        }
                    }
                    if (starColCount > 0) {
                        const float remainingStarWidth = std::max(0.0f, desired.width - allocatedWidth);
                        const float widthPerStarCol = remainingStarWidth / starColCount;
                        for (int c = std::max(0, col); c < col + colSpan && c < static_cast<int>(effectiveColumns); ++c) {
                            if (GetColumnDefinitionAt(static_cast<size_t>(c)).IsStar()) {
                                m_columnWidths[c] = std::max(m_columnWidths[c], widthPerStarCol);
                            }
                        }
                    }
                }
            }
        } else {
            const float remainingWidth = std::max(0.0f, availableSize.width - fixedWidth - autoWidth);
            for (size_t i = 0; i < effectiveColumns; ++i) {
                const GridLength definition = GetColumnDefinitionAt(i);
                if (definition.IsStar()) {
                    m_columnWidths[i] = (definition.value / totalStarWidth) * remainingWidth;
                }
            }
        }
    }

    if (totalStarHeight > 0.0f) {
        if (infiniteHeight) {
            // When available height is infinite, Star rows behave like Auto
            for (const auto& child : m_children) {
                if (!child || !child->GetIsVisible()) continue;
                auto* layoutable = child->AsLayoutable();
                if (!layoutable) continue;
                const rendering::Size desired = layoutable->GetDesiredSize();
                const int row = GetRow(child);
                const int rowSpan = std::max(1, GetRowSpan(child));
                if (rowSpan == 1 && row >= 0 && row < static_cast<int>(effectiveRows)) {
                    if (GetRowDefinitionAt(static_cast<size_t>(row)).IsStar()) {
                        m_rowHeights[row] = std::max(m_rowHeights[row], desired.height);
                    }
                } else if (rowSpan > 1) {
                    int starRowCount = 0;
                    float allocatedHeight = 0.0f;
                    for (int r = std::max(0, row); r < row + rowSpan && r < static_cast<int>(effectiveRows); ++r) {
                        const GridLength definition = GetRowDefinitionAt(static_cast<size_t>(r));
                        if (definition.IsStar()) {
                            ++starRowCount;
                        } else {
                            allocatedHeight += m_rowHeights[r];
                        }
                    }
                    if (starRowCount > 0) {
                        const float remainingStarHeight = std::max(0.0f, desired.height - allocatedHeight);
                        const float heightPerStarRow = remainingStarHeight / starRowCount;
                        for (int r = std::max(0, row); r < row + rowSpan && r < static_cast<int>(effectiveRows); ++r) {
                            if (GetRowDefinitionAt(static_cast<size_t>(r)).IsStar()) {
                                m_rowHeights[r] = std::max(m_rowHeights[r], heightPerStarRow);
                            }
                        }
                    }
                }
            }
        } else {
            const float remainingHeight = std::max(0.0f, availableSize.height - fixedHeight - autoHeight);
            for (size_t i = 0; i < effectiveRows; ++i) {
                const GridLength definition = GetRowDefinitionAt(i);
                if (definition.IsStar()) {
                    m_rowHeights[i] = (definition.value / totalStarHeight) * remainingHeight;
                }
            }
        }
    }
}

size_t Grid::GetEffectiveColumnCount() const {
    size_t count = m_columns.size();
    for (const auto& child : m_children) {
        if (!child) continue;
        const int col = std::max(0, GetColumn(child));
        const int span = std::max(1, GetColumnSpan(child));
        count = std::max(count, static_cast<size_t>(col + span));
    }
    return count;
}

size_t Grid::GetEffectiveRowCount() const {
    size_t count = m_rows.size();
    for (const auto& child : m_children) {
        if (!child) continue;
        const int row = std::max(0, GetRow(child));
        const int span = std::max(1, GetRowSpan(child));
        count = std::max(count, static_cast<size_t>(row + span));
    }
    return count;
}

GridLength Grid::GetColumnDefinitionAt(size_t index) const {
    if (index < m_columns.size()) {
        return m_columns[index];
    }
    return GridLength::Star();
}

GridLength Grid::GetRowDefinitionAt(size_t index) const {
    if (index < m_rows.size()) {
        return m_rows[index];
    }
    return GridLength::Auto();
}

void Grid::InvalidateGridLayout() {
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

} // namespace controls
} // namespace luaui
