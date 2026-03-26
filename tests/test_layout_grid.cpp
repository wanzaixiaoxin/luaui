// Layout Module - Grid Tests
#include "TestFramework.h"
#include "layouts/Grid.h"
#include "Button.h"
#include "TextBlock.h"

using namespace luaui;
using namespace luaui::controls;

// ==================== Grid Construction Tests ====================
TEST(Grid_DefaultConstruction) {
    Grid grid;
    
    ASSERT_EQ(grid.GetColumnCount(), 0u);
    ASSERT_EQ(grid.GetRowCount(), 0u);
    ASSERT_EQ(grid.GetChildCount(), 0u);
}

TEST(Grid_AddColumns) {
    Grid grid;
    
    grid.AddColumn(GridLength::Pixel(100.0f));
    ASSERT_EQ(grid.GetColumnCount(), 1u);
    
    grid.AddColumn(GridLength::Star(1.0f));
    ASSERT_EQ(grid.GetColumnCount(), 2u);
    
    grid.AddColumn(GridLength::Auto());
    ASSERT_EQ(grid.GetColumnCount(), 3u);
}

TEST(Grid_AddRows) {
    Grid grid;
    
    grid.AddRow(GridLength::Pixel(50.0f));
    ASSERT_EQ(grid.GetRowCount(), 1u);
    
    grid.AddRow(GridLength::Star(2.0f));
    ASSERT_EQ(grid.GetRowCount(), 2u);
    
    grid.AddRow(GridLength::Auto());
    ASSERT_EQ(grid.GetRowCount(), 3u);
}

TEST(Grid_ClearColumns) {
    Grid grid;
    
    grid.AddColumn(GridLength::Pixel(100.0f));
    grid.AddColumn(GridLength::Pixel(200.0f));
    ASSERT_EQ(grid.GetColumnCount(), 2u);
    
    grid.ClearColumns();
    ASSERT_EQ(grid.GetColumnCount(), 0u);
}

TEST(Grid_ClearRows) {
    Grid grid;
    
    grid.AddRow(GridLength::Pixel(50.0f));
    grid.AddRow(GridLength::Pixel(100.0f));
    ASSERT_EQ(grid.GetRowCount(), 2u);
    
    grid.ClearRows();
    ASSERT_EQ(grid.GetRowCount(), 0u);
}

// ==================== GridLength Tests ====================
TEST(GridLength_Construction) {
    GridLength autoLen = GridLength::Auto();
    ASSERT_TRUE(autoLen.IsAuto());
    ASSERT_EQ(autoLen.value, 0.0f);
    
    GridLength pixelLen = GridLength::Pixel(100.0f);
    ASSERT_TRUE(pixelLen.IsPixel());
    ASSERT_EQ(pixelLen.value, 100.0f);
    
    GridLength starLen = GridLength::Star(2.0f);
    ASSERT_TRUE(starLen.IsStar());
    ASSERT_EQ(starLen.value, 2.0f);
    
    GridLength defaultStar = GridLength::Star();
    ASSERT_TRUE(defaultStar.IsStar());
    ASSERT_EQ(defaultStar.value, 1.0f);
}

// ==================== Cell Position Tests ====================
TEST(Grid_SetGetColumn) {
    Grid grid;
    auto btn = std::make_shared<Button>();
    
    ASSERT_EQ(grid.GetColumn(btn), 0);  // Default
    
    grid.SetColumn(btn, 2);
    ASSERT_EQ(grid.GetColumn(btn), 2);
    
    grid.SetColumn(btn, 5);
    ASSERT_EQ(grid.GetColumn(btn), 5);
}

TEST(Grid_SetGetRow) {
    Grid grid;
    auto btn = std::make_shared<Button>();
    
    ASSERT_EQ(grid.GetRow(btn), 0);  // Default
    
    grid.SetRow(btn, 1);
    ASSERT_EQ(grid.GetRow(btn), 1);
    
    grid.SetRow(btn, 3);
    ASSERT_EQ(grid.GetRow(btn), 3);
}

TEST(Grid_SetGetColumnSpan) {
    Grid grid;
    auto btn = std::make_shared<Button>();
    
    ASSERT_EQ(grid.GetColumnSpan(btn), 1);  // Default
    
    grid.SetColumnSpan(btn, 2);
    ASSERT_EQ(grid.GetColumnSpan(btn), 2);
    
    grid.SetColumnSpan(btn, 3);
    ASSERT_EQ(grid.GetColumnSpan(btn), 3);
}

TEST(Grid_SetGetRowSpan) {
    Grid grid;
    auto btn = std::make_shared<Button>();
    
    ASSERT_EQ(grid.GetRowSpan(btn), 1);  // Default
    
    grid.SetRowSpan(btn, 2);
    ASSERT_EQ(grid.GetRowSpan(btn), 2);
}

// ==================== Instance Isolation Tests (fixes static variable bug) ====================
TEST(Grid_InstanceIsolation) {
    // This test verifies the fix for the static variable design defect
    Grid grid1;
    Grid grid2;
    
    auto btn1 = std::make_shared<Button>();
    auto btn2 = std::make_shared<Button>();
    
    // Set different positions in different grids
    grid1.SetColumn(btn1, 1);
    grid1.SetRow(btn1, 2);
    
    grid2.SetColumn(btn2, 3);
    grid2.SetRow(btn2, 4);
    
    // Verify isolation - grid1 should not see btn2's position
    ASSERT_EQ(grid1.GetColumn(btn1), 1);
    ASSERT_EQ(grid1.GetRow(btn1), 2);
    ASSERT_EQ(grid1.GetColumn(btn2), 0);  // Not set in grid1
    
    // Verify grid2 has its own data
    ASSERT_EQ(grid2.GetColumn(btn2), 3);
    ASSERT_EQ(grid2.GetRow(btn2), 4);
    ASSERT_EQ(grid2.GetColumn(btn1), 0);  // Not set in grid2
}

TEST(Grid_SameControlDifferentGrids) {
    // Test that the same control can have different positions in different grids
    Grid grid1;
    Grid grid2;
    
    auto btn = std::make_shared<Button>();
    
    grid1.SetColumn(btn, 0);
    grid1.SetRow(btn, 0);
    
    grid2.SetColumn(btn, 2);
    grid2.SetRow(btn, 3);
    
    // Each grid should maintain its own position data
    ASSERT_EQ(grid1.GetColumn(btn), 0);
    ASSERT_EQ(grid1.GetRow(btn), 0);
    
    ASSERT_EQ(grid2.GetColumn(btn), 2);
    ASSERT_EQ(grid2.GetRow(btn), 3);
}

// ==================== Child Management Tests ====================
TEST(Grid_AddChildren) {
    auto grid = std::make_shared<Grid>();
    
    auto btn1 = std::make_shared<Button>();
    auto btn2 = std::make_shared<Button>();
    auto text = std::make_shared<TextBlock>();
    
    grid->AddChild(btn1);
    ASSERT_EQ(grid->GetChildCount(), 1u);
    
    grid->AddChild(btn2);
    ASSERT_EQ(grid->GetChildCount(), 2u);
    
    grid->AddChild(text);
    ASSERT_EQ(grid->GetChildCount(), 3u);
}

TEST(Grid_ChildPositioning) {
    auto grid = std::make_shared<Grid>();
    grid->AddColumn(GridLength::Pixel(100.0f));
    grid->AddColumn(GridLength::Pixel(100.0f));
    grid->AddRow(GridLength::Pixel(50.0f));
    grid->AddRow(GridLength::Pixel(50.0f));
    
    auto btn1 = std::make_shared<Button>();
    auto btn2 = std::make_shared<Button>();
    auto btn3 = std::make_shared<Button>();
    
    grid->SetColumn(btn1, 0);
    grid->SetRow(btn1, 0);
    
    grid->SetColumn(btn2, 1);
    grid->SetRow(btn2, 0);
    
    grid->SetColumn(btn3, 0);
    grid->SetRow(btn3, 1);
    grid->SetColumnSpan(btn3, 2);
    
    grid->AddChild(btn1);
    grid->AddChild(btn2);
    grid->AddChild(btn3);
    
    ASSERT_EQ(grid->GetChildCount(), 3u);
    ASSERT_EQ(grid->GetColumn(btn1), 0);
    ASSERT_EQ(grid->GetRow(btn1), 0);
    ASSERT_EQ(grid->GetColumnSpan(btn3), 2);
}

// ==================== Complex Layout Tests ====================
TEST(Grid_MixedLengthTypes) {
    Grid grid;
    
    // Create a typical layout: fixed sidebar + fluid content
    grid.AddColumn(GridLength::Pixel(200.0f));  // Sidebar
    grid.AddColumn(GridLength::Star(1.0f));      // Main content
    grid.AddColumn(GridLength::Star(2.0f));      // Secondary content
    
    grid.AddRow(GridLength::Auto());
    grid.AddRow(GridLength::Star(1.0f));
    grid.AddRow(GridLength::Pixel(50.0f));       // Status bar
    
    ASSERT_EQ(grid.GetColumnCount(), 3u);
    ASSERT_EQ(grid.GetRowCount(), 3u);
}

TEST(Grid_SpanBehavior) {
    auto grid = std::make_shared<Grid>();
    
    grid->AddColumn(GridLength::Star(1.0f));
    grid->AddColumn(GridLength::Star(1.0f));
    grid->AddColumn(GridLength::Star(1.0f));
    
    grid->AddRow(GridLength::Star(1.0f));
    grid->AddRow(GridLength::Star(1.0f));
    
    auto header = std::make_shared<TextBlock>();
    grid->SetColumn(header, 0);
    grid->SetRow(header, 0);
    grid->SetColumnSpan(header, 3);  // Span all columns
    
    auto sidebar = std::make_shared<Button>();
    grid->SetColumn(sidebar, 0);
    grid->SetRow(sidebar, 1);
    
    auto content = std::make_shared<Button>();
    grid->SetColumn(content, 1);
    grid->SetRow(content, 1);
    grid->SetColumnSpan(content, 2);  // Span 2 columns
    
    grid->AddChild(header);
    grid->AddChild(sidebar);
    grid->AddChild(content);
    
    ASSERT_EQ(grid->GetColumnSpan(header), 3);
    ASSERT_EQ(grid->GetColumnSpan(content), 2);
}

// ==================== Edge Cases ====================
TEST(Grid_EmptyGrid) {
    auto grid = std::make_shared<Grid>();
    
    // Grid with no rows/columns should still work
    auto btn = std::make_shared<Button>();
    grid->AddChild(btn);
    
    ASSERT_EQ(grid->GetChildCount(), 1u);
    ASSERT_EQ(grid->GetColumnCount(), 0u);
    ASSERT_EQ(grid->GetRowCount(), 0u);
}

TEST(Grid_NullControl) {
    Grid grid;
    
    // Should handle null gracefully
    grid.SetColumn(nullptr, 1);
    grid.SetRow(nullptr, 1);
    grid.SetColumnSpan(nullptr, 2);
    grid.SetRowSpan(nullptr, 2);
    
    ASSERT_EQ(grid.GetColumn(nullptr), 0);
    ASSERT_EQ(grid.GetRow(nullptr), 0);
    ASSERT_EQ(grid.GetColumnSpan(nullptr), 1);
    ASSERT_EQ(grid.GetRowSpan(nullptr), 1);
}

TEST(Grid_NegativeSpanValues) {
    Grid grid;
    auto btn = std::make_shared<Button>();
    
    // WARNING: Negative values are stored but may cause undefined behavior
    // in layout calculations. This test documents current behavior.
    // TODO: Grid should validate and clamp negative values to 0
    grid.SetColumn(btn, -1);
    grid.SetRow(btn, -2);
    
    // Current implementation stores negative values as-is
    // This is a potential bug - should be clamped to valid range
    ASSERT_EQ(grid.GetColumn(btn), -1);
    ASSERT_EQ(grid.GetRow(btn), -2);
    
    // Test that negative spans are stored
    grid.SetColumnSpan(btn, -1);
    grid.SetRowSpan(btn, -1);
    
    ASSERT_EQ(grid.GetColumnSpan(btn), -1);
    ASSERT_EQ(grid.GetRowSpan(btn), -1);
}

TEST(Grid_BoundaryValues) {
    Grid grid;
    auto btn = std::make_shared<Button>();
    
    // Test zero values (valid)
    grid.SetColumn(btn, 0);
    grid.SetRow(btn, 0);
    grid.SetColumnSpan(btn, 1);
    grid.SetRowSpan(btn, 1);
    
    ASSERT_EQ(grid.GetColumn(btn), 0);
    ASSERT_EQ(grid.GetRow(btn), 0);
    ASSERT_EQ(grid.GetColumnSpan(btn), 1);
    ASSERT_EQ(grid.GetRowSpan(btn), 1);
    
    // Test large values
    grid.SetColumn(btn, 1000000);
    grid.SetRow(btn, 1000000);
    
    ASSERT_EQ(grid.GetColumn(btn), 1000000);
    ASSERT_EQ(grid.GetRow(btn), 1000000);
}

// ==================== Performance Tests ====================
TEST(Grid_LargeGrid) {
    auto grid = std::make_shared<Grid>();
    
    // Create a 10x10 grid
    for (int i = 0; i < 10; ++i) {
        grid->AddColumn(GridLength::Star(1.0f));
        grid->AddRow(GridLength::Star(1.0f));
    }
    
    ASSERT_EQ(grid->GetColumnCount(), 10u);
    ASSERT_EQ(grid->GetRowCount(), 10u);
    
    // Add 100 buttons
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            auto btn = std::make_shared<Button>();
            grid->SetColumn(btn, col);
            grid->SetRow(btn, row);
            grid->AddChild(btn);
        }
    }
    
    ASSERT_EQ(grid->GetChildCount(), 100u);
}

// ==================== Memory Safety Tests ====================
TEST(Grid_ControlDestruction) {
    Grid grid;
    
    {
        auto btn = std::make_shared<Button>();
        grid.SetColumn(btn, 5);
        grid.SetRow(btn, 3);
        
        ASSERT_EQ(grid.GetColumn(btn), 5);
        ASSERT_EQ(grid.GetRow(btn), 3);
        
        // btn goes out of scope here
    }
    
    // Grid should still be valid after control destruction
    // (CellInfo remains but control is gone - this is expected behavior)
    ASSERT_EQ(grid.GetColumnCount(), 0u);
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
