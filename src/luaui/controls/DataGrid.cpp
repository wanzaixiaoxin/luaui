#include "DataGrid.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/IRenderable.h"
#include "Interfaces/ILayoutable.h"
#include "IRenderContext.h"

namespace luaui {
namespace controls {

// ============================================================================
// DataGridColumn
// ============================================================================
DataGridColumn::DataGridColumn() {}

DataGridColumn::DataGridColumn(const std::wstring& header) : m_header(header) {}

// ============================================================================
// DataGridCell
// ============================================================================
DataGridCell::DataGridCell() {}

void DataGridCell::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
}

void DataGridCell::SetValue(const std::any& value) {
    m_value = value;
    m_text = FormatValue(value);
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void DataGridCell::SetText(const std::wstring& text) {
    m_text = text;
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

std::wstring DataGridCell::FormatValue(const std::any& value) {
    if (!value.has_value()) return L"";
    
    // 简化：处理常见类型
    try {
        if (value.type() == typeid(std::wstring)) {
            return std::any_cast<std::wstring>(value);
        }
        if (value.type() == typeid(std::string)) {
            std::string s = std::any_cast<std::string>(value);
            return std::wstring(s.begin(), s.end());
        }
        if (value.type() == typeid(int)) {
            return std::to_wstring(std::any_cast<int>(value));
        }
        if (value.type() == typeid(double)) {
            return std::to_wstring(std::any_cast<double>(value));
        }
        if (value.type() == typeid(float)) {
            return std::to_wstring(std::any_cast<float>(value));
        }
        if (value.type() == typeid(bool)) {
            return std::any_cast<bool>(value) ? L"Yes" : L"No";
        }
    } catch (...) {
        return L"";
    }
    return L"";
}

void DataGridCell::SetIsSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        UpdateVisualState();
    }
}

void DataGridCell::SetIsEditing(bool editing) {
    if (m_isEditing != editing) {
        m_isEditing = editing;
        UpdateVisualState();
    }
}

void DataGridCell::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void DataGridCell::OnMouseEnter() {
    m_isHovered = true;
    UpdateVisualState();
}

void DataGridCell::OnMouseLeave() {
    m_isHovered = false;
    UpdateVisualState();
}

void DataGridCell::OnClick() {
    if (m_row && m_row->GetDataGrid()) {
        m_row->GetDataGrid()->OnCellClicked(this);
    }
}

rendering::Size DataGridCell::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    float width = m_column ? m_column->GetActualWidth() : 100.0f;
    return rendering::Size(width, m_row ? m_row->GetHeight() : 32.0f);
}

void DataGridCell::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景
    rendering::Color bgColor = m_normalBg;
    if (m_isSelected) {
        bgColor = m_selectedBg;
    } else if (m_isHovered) {
        bgColor = m_hoverBg;
    }
    
    if (bgColor.a > 0) {
        auto bgBrush = context->CreateSolidColorBrush(bgColor);
        if (bgBrush) {
            context->FillRectangle(rect, bgBrush.get());
        }
    }
    
    // 绘制文本
    if (!m_text.empty()) {
        rendering::Color textColor = m_isSelected ? m_selectedTextColor : m_textColor;
        auto textBrush = context->CreateSolidColorBrush(textColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        
        if (textBrush && textFormat) {
            rendering::Point textPos(rect.x + m_padding, 
                                     rect.y + (rect.height - m_fontSize) / 2);
            context->DrawTextString(m_text, textFormat.get(), textPos, textBrush.get());
        }
    }
}

// ============================================================================
// DataGridRow
// ============================================================================
DataGridRow::DataGridRow() {}

void DataGridRow::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
}

void DataGridRow::AddCell(const std::shared_ptr<DataGridCell>& cell) {
    if (!cell) return;
    
    cell->SetRow(this);
    m_cells.push_back(cell);
    // Note: DataGridRow inherits from Control, not Panel
    // Cells are rendered by the row's custom render logic, not as child controls
}

void DataGridRow::ClearCells() {
    m_cells.clear();
}

std::shared_ptr<DataGridCell> DataGridRow::GetCell(size_t index) {
    if (index < m_cells.size()) {
        return m_cells[index];
    }
    return nullptr;
}

void DataGridRow::SetIsSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        
        // 更新所有单元格的选中状态
        for (auto& cell : m_cells) {
            cell->SetIsSelected(selected);
        }
        
        UpdateVisualState();
        
        if (m_dataGrid && selected) {
            m_dataGrid->OnRowSelected(this);
        }
    }
}

void DataGridRow::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

void DataGridRow::OnMouseEnter() {
    m_isHovered = true;
    if (!m_isSelected) {
        for (auto& cell : m_cells) {
            cell->SetIsHovered(true);
        }
    }
}

void DataGridRow::OnMouseLeave() {
    m_isHovered = false;
    for (auto& cell : m_cells) {
        cell->SetIsHovered(false);
    }
}

void DataGridRow::OnClick() {
    SetIsSelected(true);
}

rendering::Size DataGridRow::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    
    float totalWidth = 0;
    for (auto& cell : m_cells) {
        if (auto* layoutable = cell->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(0, m_height);
            layoutable->Measure(constraint);
            auto size = layoutable->GetDesiredSize();
            totalWidth += size.width;
        }
    }
    
    return rendering::Size(totalWidth, m_height);
}

void DataGridRow::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    // 行背景由单元格绘制，这里只绘制行边框（可选）
    // 实际应用中可能不需要
    (void)context;
}

// ============================================================================
// DataGrid
// ============================================================================
DataGrid::DataGrid() {}

void DataGrid::InitializeComponents() {
    Panel::InitializeComponents();
    
    // 默认大小
    if (auto* layout = GetLayout()) {
        layout->SetMinWidth(200);
        layout->SetMinHeight(150);
    }
}

void DataGrid::AddColumn(const std::shared_ptr<DataGridColumn>& column) {
    if (!column) return;
    
    m_columns.push_back(column);
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void DataGrid::RemoveColumn(const std::shared_ptr<DataGridColumn>& column) {
    auto it = std::find(m_columns.begin(), m_columns.end(), column);
    if (it != m_columns.end()) {
        m_columns.erase(it);
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void DataGrid::ClearColumns() {
    m_columns.clear();
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

std::shared_ptr<DataGridColumn> DataGrid::GetColumn(size_t index) {
    if (index < m_columns.size()) {
        return m_columns[index];
    }
    return nullptr;
}

std::shared_ptr<DataGridColumn> DataGrid::GetColumn(const std::wstring& header) {
    for (auto& col : m_columns) {
        if (col->GetHeader() == header) {
            return col;
        }
    }
    return nullptr;
}

void DataGrid::AddRow(const std::shared_ptr<DataGridRow>& row) {
    if (!row) return;
    
    row->SetDataGrid(this);
    row->SetIndex(static_cast<int>(m_rows.size()));
    
    // 自动创建缺失的单元格
    while (row->GetCellCount() < m_columns.size()) {
        auto cell = std::make_shared<DataGridCell>();
        cell->SetColumn(GetColumn(row->GetCellCount()));
        row->AddCell(cell);
    }
    
    m_rows.push_back(row);
    Panel::AddChild(row);
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

void DataGrid::RemoveRow(const std::shared_ptr<DataGridRow>& row) {
    auto it = std::find(m_rows.begin(), m_rows.end(), row);
    if (it != m_rows.end()) {
        // 从选中列表中移除
        auto selectedIt = std::find(m_selectedRows.begin(), m_selectedRows.end(), row);
        if (selectedIt != m_selectedRows.end()) {
            m_selectedRows.erase(selectedIt);
        }
        
        Panel::RemoveChild(*it);
        m_rows.erase(it);
        
        // 更新索引
        for (size_t i = 0; i < m_rows.size(); ++i) {
            m_rows[i]->SetIndex(static_cast<int>(i));
        }
        
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void DataGrid::ClearRows() {
    for (auto& row : m_rows) {
        Panel::RemoveChild(row);
    }
    m_rows.clear();
    m_selectedRows.clear();
    
    if (auto* layout = GetLayout()) {
        layout->InvalidateMeasure();
    }
}

std::shared_ptr<DataGridRow> DataGrid::GetRow(size_t index) {
    if (index < m_rows.size()) {
        return m_rows[index];
    }
    return nullptr;
}

std::shared_ptr<DataGridRow> DataGrid::GetSelectedRow() const {
    if (!m_selectedRows.empty()) {
        return m_selectedRows[0];
    }
    return nullptr;
}

std::vector<std::shared_ptr<DataGridRow>> DataGrid::GetSelectedRows() const {
    return m_selectedRows;
}

void DataGrid::SetSelectedRow(const std::shared_ptr<DataGridRow>& row) {
    ClearSelection();
    if (row) {
        row->SetIsSelected(true);
    }
}

void DataGrid::SelectRow(int index) {
    if (index >= 0 && index < static_cast<int>(m_rows.size())) {
        SetSelectedRow(m_rows[index]);
    }
}

void DataGrid::ClearSelection() {
    for (auto& row : m_selectedRows) {
        row->SetIsSelected(false);
    }
    m_selectedRows.clear();
}

void DataGrid::OnRowSelected(DataGridRow* row) {
    if (!row) return;
    
    auto rowPtr = std::static_pointer_cast<DataGridRow>(row->shared_from_this());
    
    if (m_selectionMode == SelectionMode::Single) {
        // 单选模式：清除其他选择
        for (auto& selectedRow : m_selectedRows) {
            if (selectedRow.get() != row) {
                selectedRow->SetIsSelected(false);
            }
        }
        m_selectedRows.clear();
        m_selectedRows.push_back(rowPtr);
    } else if (m_selectionMode == SelectionMode::Multiple || 
               m_selectionMode == SelectionMode::Extended) {
        // 多选模式
        auto it = std::find(m_selectedRows.begin(), m_selectedRows.end(), rowPtr);
        if (it == m_selectedRows.end()) {
            m_selectedRows.push_back(rowPtr);
        }
    }
    
    SelectionChanged.Invoke(this, row);
}

void DataGrid::OnCellClicked(DataGridCell* cell) {
    CellClick.Invoke(this, cell);
}

void DataGrid::CalculateColumnWidths(float totalWidth) {
    if (m_columns.empty()) return;
    
    // 简化实现：平均分配宽度
    float colWidth = totalWidth / m_columns.size();
    for (auto& col : m_columns) {
        col->SetActualWidth(colWidth);
    }
}

int DataGrid::HitTestColumnHeader(float x) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    float currentX = rect.x - m_scrollOffsetX;
    for (size_t i = 0; i < m_columns.size(); ++i) {
        float colWidth = m_columns[i]->GetActualWidth();
        if (x >= currentX && x < currentX + colWidth) {
            return static_cast<int>(i);
        }
        currentX += colWidth;
    }
    return -1;
}

int DataGrid::HitTestRow(float y) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    float rowStartY = rect.y + m_headerHeight - m_scrollOffsetY;
    
    for (size_t i = 0; i < m_rows.size(); ++i) {
        if (y >= rowStartY && y < rowStartY + m_rowHeight) {
            return static_cast<int>(i);
        }
        rowStartY += m_rowHeight;
    }
    return -1;
}

void DataGrid::Refresh() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

rendering::Size DataGrid::OnMeasureChildren(const rendering::Size& availableSize) {
    // 计算列宽
    CalculateColumnWidths(availableSize.width);
    
    // 测量行
    for (auto& row : m_rows) {
        if (auto* layoutable = row->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = availableSize;
            layoutable->Measure(constraint);
        }
        
        // 测量单元格
        for (size_t i = 0; i < row->GetCellCount() && i < m_columns.size(); ++i) {
            if (auto cell = row->GetCell(i)) {
                if (auto* cellLayoutable = cell->AsLayoutable()) {
                    cell->SetColumn(m_columns[i]);
                    interfaces::LayoutConstraint constraint;
                    constraint.available = rendering::Size(m_columns[i]->GetActualWidth(), m_rowHeight);
                    cellLayoutable->Measure(constraint);
                }
            }
        }
    }
    
    return availableSize;
}

rendering::Size DataGrid::OnArrangeChildren(const rendering::Size& finalSize) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    // 排列行
    float y = rect.y + m_headerHeight - m_scrollOffsetY;
    for (auto& row : m_rows) {
        if (auto* layoutable = row->AsLayoutable()) {
            layoutable->Arrange(rendering::Rect(rect.x, y, finalSize.width, m_rowHeight));
        }
        
        // 排列单元格
        float x = rect.x - m_scrollOffsetX;
        for (size_t i = 0; i < row->GetCellCount() && i < m_columns.size(); ++i) {
            if (auto cell = row->GetCell(i)) {
                if (auto* cellLayoutable = cell->AsLayoutable()) {
                    float colWidth = m_columns[i]->GetActualWidth();
                    cellLayoutable->Arrange(rendering::Rect(x, y, colWidth, m_rowHeight));
                    x += colWidth;
                }
            }
        }
        
        y += m_rowHeight;
    }
    
    return finalSize;
}

void DataGrid::OnRenderChildren(rendering::IRenderContext* context) {
    if (!context) return;
    
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    // 绘制边框
    auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
    if (borderBrush) {
        context->DrawRectangle(rect, borderBrush.get(), 1.0f);
    }
    
    // 绘制列头
    RenderHeader(context);
    
    // 绘制行（可见区域）
    float rowStartY = rect.y + m_headerHeight;
    float contentHeight = rect.height - m_headerHeight;
    
    for (size_t i = 0; i < m_rows.size(); ++i) {
        float rowY = rowStartY + i * m_rowHeight - m_scrollOffsetY;
        
        // 可见性测试
        if (rowY + m_rowHeight < rowStartY || rowY > rowStartY + contentHeight) {
            continue;
        }
        
        // 绘制交替行背景
        if (m_alternatingRowBackground && i % 2 == 1) {
            auto altBrush = context->CreateSolidColorBrush(rendering::Color::FromHex(0xF5F5F5));
            if (altBrush) {
                context->FillRectangle(
                    rendering::Rect(rect.x + 1, rowY, rect.width - 2, m_rowHeight),
                    altBrush.get());
            }
        }
        
        // 渲染单元格
        for (size_t j = 0; j < m_rows[i]->GetCellCount(); ++j) {
            if (auto cell = m_rows[i]->GetCell(j)) {
                if (auto* cellRenderable = cell->AsRenderable()) {
                    cellRenderable->Render(context);
                }
            }
        }
        
        // 绘制行分隔线
        auto lineBrush = context->CreateSolidColorBrush(m_gridLineColor);
        if (lineBrush) {
            context->DrawLine(rendering::Point(rect.x, rowY + m_rowHeight),
                              rendering::Point(rect.x + rect.width, rowY + m_rowHeight),
                              lineBrush.get(), 1.0f);
        }
    }
    
    // 绘制列分隔线
    if (!m_columns.empty()) {
        auto lineBrush = context->CreateSolidColorBrush(m_gridLineColor);
        if (lineBrush) {
            float x = rect.x - m_scrollOffsetX;
            for (size_t i = 0; i < m_columns.size(); ++i) {
                x += m_columns[i]->GetActualWidth();
                if (x > rect.x && x < rect.x + rect.width) {
                    context->DrawLine(rendering::Point(x, rect.y + m_headerHeight),
                                      rendering::Point(x, rect.y + rect.height),
                                      lineBrush.get(), 1.0f);
                }
            }
        }
    }
}

void DataGrid::RenderHeader(rendering::IRenderContext* context) {
    if (!context || m_columns.empty()) return;
    
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    rendering::Rect headerRect(rect.x, rect.y, rect.width, m_headerHeight);
    
    // 绘制列头背景
    auto bgBrush = context->CreateSolidColorBrush(m_headerBg);
    if (bgBrush) {
        context->FillRectangle(headerRect, bgBrush.get());
    }
    
    // 绘制列头边框
    auto borderBrush = context->CreateSolidColorBrush(m_headerBorder);
    if (borderBrush) {
        context->DrawRectangle(headerRect, borderBrush.get(), 1.0f);
    }
    
    // 绘制列标题
    auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", 14.0f);
    auto textBrush = context->CreateSolidColorBrush(rendering::Color::Black());
    
    if (textFormat && textBrush) {
        float x = rect.x - m_scrollOffsetX;
        for (auto& col : m_columns) {
            if (!col->GetIsVisible()) continue;
            
            float colWidth = col->GetActualWidth();
            
            // 绘制列分隔线
            if (borderBrush) {
                context->DrawLine(rendering::Point(x + colWidth, rect.y),
                                  rendering::Point(x + colWidth, rect.y + m_headerHeight),
                                  borderBrush.get(), 1.0f);
            }
            
            // 绘制标题文本
            if (!col->GetHeader().empty()) {
                rendering::Point textPos(x + 8, rect.y + (m_headerHeight - 14) / 2);
                context->DrawTextString(col->GetHeader(), textFormat.get(), 
                                        textPos, textBrush.get());
            }
            
            x += colWidth;
        }
    }
}

void DataGrid::OnMouseMove(MouseEventArgs& args) {
    (void)args;
    // 处理悬停效果
}

void DataGrid::OnMouseDown(MouseEventArgs& args) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    // 检查是否点击列头
    if (args.y < rect.y + m_headerHeight) {
        int colIndex = HitTestColumnHeader(args.x);
        if (colIndex >= 0 && colIndex < static_cast<int>(m_columns.size())) {
            ColumnHeaderClick.Invoke(this, m_columns[colIndex].get());
        }
    } else {
        // 检查是否点击行
        int rowIndex = HitTestRow(args.y);
        if (rowIndex >= 0 && rowIndex < static_cast<int>(m_rows.size())) {
            SelectRow(rowIndex);
        }
    }
    
    args.Handled = true;
}

} // namespace controls
} // namespace luaui
