#pragma once

#include "Panel.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace luaui {
namespace interfaces {
    class IControl;
}

namespace controls {

/**
 * @brief GridLength - 网格长度定义
 */
struct GridLength {
    enum Type { AutoType, PixelType, StarType };

    Type type;
    float value;

    GridLength() : type(AutoType), value(0.0f) {}
    explicit GridLength(float pixels) : type(PixelType), value(pixels) {}
    GridLength(Type t, float v) : type(t), value(v) {}

    static GridLength Auto() { return GridLength(); }
    static GridLength Pixel(float pixels) { return GridLength(pixels); }
    static GridLength Star(float stars = 1.0f) { return GridLength(StarType, stars); }

    bool IsAuto() const { return type == AutoType; }
    bool IsPixel() const { return type == PixelType; }
    bool IsStar() const { return type == StarType; }
};

/**
 * @brief Grid - 网格布局面板（新架构）
 */
class Grid : public Panel {
public:
    Grid();

    std::string GetTypeName() const override { return "Grid"; }

    void AddColumn(const GridLength& width);
    void SetColumnDefinition(size_t index, const GridLength& width);
    void ClearColumns();
    size_t GetColumnCount() const { return m_columns.size(); }

    void AddRow(const GridLength& height);
    void SetRowDefinition(size_t index, const GridLength& height);
    void ClearRows();
    size_t GetRowCount() const { return m_rows.size(); }

    void SetColumn(const std::shared_ptr<interfaces::IControl>& control, int column);
    void SetRow(const std::shared_ptr<interfaces::IControl>& control, int row);
    void SetColumnSpan(const std::shared_ptr<interfaces::IControl>& control, int span);
    void SetRowSpan(const std::shared_ptr<interfaces::IControl>& control, int span);
    int GetColumn(const std::shared_ptr<interfaces::IControl>& control) const;
    int GetRow(const std::shared_ptr<interfaces::IControl>& control) const;
    int GetColumnSpan(const std::shared_ptr<interfaces::IControl>& control) const;
    int GetRowSpan(const std::shared_ptr<interfaces::IControl>& control) const;

protected:
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    struct CellInfo {
        int column = 0;
        int row = 0;
        int columnSpan = 1;
        int rowSpan = 1;
    };

    std::vector<GridLength> m_columns;
    std::vector<GridLength> m_rows;
    std::vector<float> m_columnWidths;
    std::vector<float> m_rowHeights;
    std::unordered_map<ControlID, CellInfo> m_cellInfo;

    void CalculateSizes(const rendering::Size& availableSize);
    size_t GetEffectiveColumnCount() const;
    size_t GetEffectiveRowCount() const;
    GridLength GetColumnDefinitionAt(size_t index) const;
    GridLength GetRowDefinitionAt(size_t index) const;
    void InvalidateGridLayout();
};

} // namespace controls
} // namespace luaui
