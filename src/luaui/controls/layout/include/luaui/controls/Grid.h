#pragma once
#include "LayoutPanel.h"
#include <vector>

namespace luaui {
namespace controls {

// Grid unit type
enum class GridUnitType { Auto, Pixel, Star };

// Grid length definition
struct GridLength {
    float Value = 0;
    GridUnitType Type = GridUnitType::Auto;
    
    GridLength() = default;
    explicit GridLength(float value) : Value(value), Type(GridUnitType::Pixel) {}
    GridLength(float value, GridUnitType type) : Value(value), Type(type) {}
    
    static GridLength Auto() { return GridLength(0, GridUnitType::Auto); }
    static GridLength Star(float value = 1) { return GridLength(value, GridUnitType::Star); }
    static GridLength Pixel(float value) { return GridLength(value, GridUnitType::Pixel); }
};

struct RowDefinition {
    GridLength Height = GridLength::Auto();
    float MinHeight = 0;
    float MaxHeight = std::numeric_limits<float>::max();
    float ActualHeight = 0;
};

struct ColumnDefinition {
    GridLength Width = GridLength::Auto();
    float MinWidth = 0;
    float MaxWidth = std::numeric_limits<float>::max();
    float ActualWidth = 0;
};

// Grid - Arranges children in rows and columns
class Grid : public Panel {
public:
    Grid() = default;
    
    std::string GetTypeName() const override { return "Grid"; }
    
    // Show grid lines (for debugging)
    bool ShowGridLines = false;
    
    // Row/Column definitions
    void AddRow(const GridLength& height = GridLength::Auto(), 
                float minHeight = 0, 
                float maxHeight = std::numeric_limits<float>::max());
    void AddColumn(const GridLength& width = GridLength::Auto(), 
                   float minWidth = 0, 
                   float maxWidth = std::numeric_limits<float>::max());
    void ClearRows() { m_rows.clear(); }
    void ClearColumns() { m_columns.clear(); }
    
    const std::vector<RowDefinition>& GetRows() const { return m_rows; }
    const std::vector<ColumnDefinition>& GetColumns() const { return m_columns; }
    
    // Attached properties
    static void SetRow(Control* control, int row);
    static void SetColumn(Control* control, int column);
    static void SetRowSpan(Control* control, int span);
    static void SetColumnSpan(Control* control, int span);
    static int GetRow(Control* control);
    static int GetColumn(Control* control);
    static int GetRowSpan(Control* control);
    static int GetColumnSpan(Control* control);
    
protected:
    Size MeasureOverride(const Size& availableSize) override;
    Size ArrangeOverride(const Size& finalSize) override;
    
private:
    std::vector<RowDefinition> m_rows;
    std::vector<ColumnDefinition> m_columns;
    
    void CalculateRowHeights(float availableHeight);
    void CalculateColumnWidths(float availableWidth);
};

} // namespace controls
} // namespace luaui
