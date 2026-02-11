#pragma once
#include "Panel.h"
#include <unordered_map>
#include <memory>
#include <vector>

namespace luaui {
namespace controls {

/**
 * @brief GridLength - 网格长度定义
 */
struct GridLength {
    enum Type { AutoType, PixelType, StarType };
    
    Type type;
    float value;
    
    GridLength() : type(AutoType), value(0) {}
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
    
    // 列定义
    void AddColumn(const GridLength& width);
    void ClearColumns() { m_columns.clear(); }
    size_t GetColumnCount() const { return m_columns.size(); }
    
    // 行定义
    void AddRow(const GridLength& height);
    void ClearRows() { m_rows.clear(); }
    size_t GetRowCount() const { return m_rows.size(); }
    
    // 附加属性
    static void SetColumn(const std::shared_ptr<IControl>& control, int column);
    static void SetRow(const std::shared_ptr<IControl>& control, int row);
    static void SetColumnSpan(const std::shared_ptr<IControl>& control, int span);
    static void SetRowSpan(const std::shared_ptr<IControl>& control, int span);
    static int GetColumn(const std::shared_ptr<IControl>& control);
    static int GetRow(const std::shared_ptr<IControl>& control);
    static int GetColumnSpan(const std::shared_ptr<IControl>& control);
    static int GetRowSpan(const std::shared_ptr<IControl>& control);

protected:
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    std::vector<GridLength> m_columns;
    std::vector<GridLength> m_rows;
    std::vector<float> m_columnWidths;
    std::vector<float> m_rowHeights;
    
    // 存储子控件的网格位置
    struct CellInfo {
        int column = 0;
        int row = 0;
        int columnSpan = 1;
        int rowSpan = 1;
    };
    static std::unordered_map<ControlID, CellInfo> s_cellInfo;
    
    void CalculateSizes(const rendering::Size& availableSize);
};

} // namespace controls
} // namespace luaui
