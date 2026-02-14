#pragma once

#include "Panel.h"
#include "../rendering/Types.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <any>

namespace luaui {
namespace controls {

// 前向声明
class DataGrid;
class DataGridColumn;
class DataGridRow;
class DataGridCell;

/**
 * @brief DataGridColumn 表格列定义
 */
class DataGridColumn {
public:
    DataGridColumn();
    explicit DataGridColumn(const std::wstring& header);
    
    // 列标题
    std::wstring GetHeader() const { return m_header; }
    void SetHeader(const std::wstring& header) { m_header = header; }
    
    // 绑定字段名
    std::wstring GetBindingPath() const { return m_bindingPath; }
    void SetBindingPath(const std::wstring& path) { m_bindingPath = path; }
    
    // 列宽（像素或 * 表示比例）
    std::wstring GetWidth() const { return m_width; }
    void SetWidth(const std::wstring& width) { m_width = width; }
    float GetActualWidth() const { return m_actualWidth; }
    void SetActualWidth(float width) { m_actualWidth = width; }
    
    // 最小/最大宽度
    float GetMinWidth() const { return m_minWidth; }
    void SetMinWidth(float width) { m_minWidth = width; }
    float GetMaxWidth() const { return m_maxWidth; }
    void SetMaxWidth(float width) { m_maxWidth = width; }
    
    // 是否可见
    bool GetIsVisible() const { return m_isVisible; }
    void SetIsVisible(bool visible) { m_isVisible = visible; }
    
    // 是否可排序
    bool GetCanSort() const { return m_canSort; }
    void SetCanSort(bool canSort) { m_canSort = canSort; }
    
    // 是否只读
    bool GetIsReadOnly() const { return m_isReadOnly; }
    void SetIsReadOnly(bool readOnly) { m_isReadOnly = readOnly; }
    
    // 单元格模板（可选）
    // std::shared_ptr<DataTemplate> CellTemplate;
    
    // 文本格式化
    std::wstring GetStringFormat() const { return m_stringFormat; }
    void SetStringFormat(const std::wstring& format) { m_stringFormat = format; }

private:
    std::wstring m_header;
    std::wstring m_bindingPath;
    std::wstring m_width = L"100";  // 默认100像素
    float m_actualWidth = 100.0f;
    float m_minWidth = 20.0f;
    float m_maxWidth = 1000.0f;
    bool m_isVisible = true;
    bool m_canSort = true;
    bool m_isReadOnly = true;
    std::wstring m_stringFormat;
};

/**
 * @brief DataGridCell 表格单元格
 */
class DataGridCell : public luaui::Control {
public:
    DataGridCell();
    
    std::string GetTypeName() const override { return "DataGridCell"; }
    
    // 所属列
    std::shared_ptr<DataGridColumn> GetColumn() const { return m_column; }
    void SetColumn(const std::shared_ptr<DataGridColumn>& column) { m_column = column; }
    
    // 所属行
    DataGridRow* GetRow() const { return m_row; }
    void SetRow(DataGridRow* row) { m_row = row; }
    
    // 单元格值
    std::any GetValue() const { return m_value; }
    void SetValue(const std::any& value);
    
    // 显示文本
    std::wstring GetText() const { return m_text; }
    void SetText(const std::wstring& text);
    
    // 是否选中
    bool GetIsSelected() const { return m_isSelected; }
    void SetIsSelected(bool selected);
    
    // 是否编辑中
    bool GetIsEditing() const { return m_isEditing; }
    void SetIsEditing(bool editing);

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnClick() override;

private:
    void UpdateVisualState();
    std::wstring FormatValue(const std::any& value);
    
    std::shared_ptr<DataGridColumn> m_column;
    DataGridRow* m_row = nullptr;
    std::any m_value;
    std::wstring m_text;
    
    bool m_isSelected = false;
    bool m_isHovered = false;
    bool m_isEditing = false;
    
    // 外观
    float m_padding = 8.0f;
    float m_fontSize = 14.0f;
    
    // 颜色
    rendering::Color m_normalBg = rendering::Color::Transparent();
    rendering::Color m_hoverBg = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_selectedBg = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_textColor = rendering::Color::Black();
    rendering::Color m_selectedTextColor = rendering::Color::White();
};

/**
 * @brief DataGridRow 表格行
 */
class DataGridRow : public luaui::Control,
                    public std::enable_shared_from_this<DataGridRow> {
public:
    DataGridRow();
    
    std::string GetTypeName() const override { return "DataGridRow"; }
    
    // 数据项
    std::any GetItem() const { return m_item; }
    void SetItem(const std::any& item) { m_item = item; }
    
    // 单元格
    void AddCell(const std::shared_ptr<DataGridCell>& cell);
    void ClearCells();
    size_t GetCellCount() const { return m_cells.size(); }
    std::shared_ptr<DataGridCell> GetCell(size_t index);
    
    // 是否选中
    bool GetIsSelected() const { return m_isSelected; }
    void SetIsSelected(bool selected);
    
    // 行索引
    int GetIndex() const { return m_index; }
    void SetIndex(int index) { m_index = index; }
    
    // 行高
    float GetHeight() const { return m_height; }
    void SetHeight(float height) { m_height = height; }
    
    // 所属 DataGrid
    DataGrid* GetDataGrid() const { return m_dataGrid; }
    void SetDataGrid(DataGrid* grid) { m_dataGrid = grid; }

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    void OnRender(rendering::IRenderContext* context) override;
    
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnClick() override;

private:
    void UpdateVisualState();
    
    std::any m_item;
    std::vector<std::shared_ptr<DataGridCell>> m_cells;
    DataGrid* m_dataGrid = nullptr;
    
    bool m_isSelected = false;
    bool m_isHovered = false;
    int m_index = -1;
    float m_height = 32.0f;
    
    rendering::Color m_normalBg = rendering::Color::Transparent();
    rendering::Color m_hoverBg = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_selectedBg = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_alternateBg = rendering::Color::FromHex(0xF5F5F5); // 交替行背景
};

/**
 * @brief DataGrid 数据表格（新架构）
 * 
 * 功能：
 * - 列定义（DataGridColumn）
 * - 行数据绑定
 * - 单元格/行选中
 * - 列头点击排序（简化版）
 * - 滚动支持
 * - 交替行背景
 */
class DataGrid : public Panel {
public:
    // 选择模式
    enum class SelectionMode {
        Single,      // 单选（默认）
        Multiple,    // 多选
        Extended,    // 扩展选择（Ctrl/Shift）
        None         // 不选
    };
    
    DataGrid();
    
    std::string GetTypeName() const override { return "DataGrid"; }
    
    // 列操作
    void AddColumn(const std::shared_ptr<DataGridColumn>& column);
    void RemoveColumn(const std::shared_ptr<DataGridColumn>& column);
    void ClearColumns();
    size_t GetColumnCount() const { return m_columns.size(); }
    std::shared_ptr<DataGridColumn> GetColumn(size_t index);
    std::shared_ptr<DataGridColumn> GetColumn(const std::wstring& header);
    
    // 数据操作
    template<typename T>
    void SetItemsSource(const std::vector<T>& items);
    void ClearItems();
    size_t GetItemCount() const { return m_rows.size(); }
    
    // 行操作
    void AddRow(const std::shared_ptr<DataGridRow>& row);
    void RemoveRow(const std::shared_ptr<DataGridRow>& row);
    void ClearRows();
    std::shared_ptr<DataGridRow> GetRow(size_t index);
    
    // 选中
    SelectionMode GetSelectionMode() const { return m_selectionMode; }
    void SetSelectionMode(SelectionMode mode) { m_selectionMode = mode; }
    
    std::shared_ptr<DataGridRow> GetSelectedRow() const;
    std::vector<std::shared_ptr<DataGridRow>> GetSelectedRows() const;
    void SetSelectedRow(const std::shared_ptr<DataGridRow>& row);
    void SelectRow(int index);
    void ClearSelection();
    
    // 事件
    luaui::Delegate<DataGrid*, DataGridRow*> SelectionChanged;
    luaui::Delegate<DataGrid*, DataGridCell*> CellClick;
    luaui::Delegate<DataGrid*, DataGridColumn*> ColumnHeaderClick;
    
    // 外观
    bool GetAutoGenerateColumns() const { return m_autoGenerateColumns; }
    void SetAutoGenerateColumns(bool autoGenerate) { m_autoGenerateColumns = autoGenerate; }
    
    bool GetIsReadOnly() const { return m_isReadOnly; }
    void SetIsReadOnly(bool readOnly) { m_isReadOnly = readOnly; }
    
    bool GetAlternatingRowBackground() const { return m_alternatingRowBackground; }
    void SetAlternatingRowBackground(bool enable) { m_alternatingRowBackground = enable; }
    
    float GetRowHeight() const { return m_rowHeight; }
    void SetRowHeight(float height) { m_rowHeight = height; }
    
    float GetHeaderHeight() const { return m_headerHeight; }
    void SetHeaderHeight(float height) { m_headerHeight = height; }
    
    // 刷新
    void Refresh();

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;
    void OnRenderChildren(rendering::IRenderContext* context) override;
    
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseDown(MouseEventArgs& args) override;

private:
    friend class DataGridRow;
    friend class DataGridCell;
    
    void OnRowSelected(DataGridRow* row);
    void OnCellClicked(DataGridCell* cell);
    void CalculateColumnWidths(float totalWidth);
    void RenderHeader(rendering::IRenderContext* context);
    int HitTestColumnHeader(float x);
    int HitTestRow(float y);
    
    std::vector<std::shared_ptr<DataGridColumn>> m_columns;
    std::vector<std::shared_ptr<DataGridRow>> m_rows;
    std::vector<std::shared_ptr<DataGridRow>> m_selectedRows;
    
    SelectionMode m_selectionMode = SelectionMode::Single;
    bool m_autoGenerateColumns = true;
    bool m_isReadOnly = true;
    bool m_alternatingRowBackground = true;
    
    float m_rowHeight = 32.0f;
    float m_headerHeight = 32.0f;
    float m_scrollOffsetX = 0.0f;
    float m_scrollOffsetY = 0.0f;
    
    // 颜色
    rendering::Color m_headerBg = rendering::Color::FromHex(0xF5F5F5);
    rendering::Color m_headerBorder = rendering::Color::FromHex(0xCCCCCC);
    rendering::Color m_gridLineColor = rendering::Color::FromHex(0xE0E0E0);
    rendering::Color m_borderColor = rendering::Color::FromHex(0xCCCCCC);
    
    // 列宽拖动（简化：暂不支持）
    int m_resizingColumn = -1;
    float m_resizeStartX = 0;
};

} // namespace controls
} // namespace luaui
