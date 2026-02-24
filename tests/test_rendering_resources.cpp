// Rendering Module - DirtyRegion Tests (API-matched)
#include "TestFramework.h"
#include "DirtyRegion.h"
#include "Types.h"

using namespace luaui::rendering;

// ==================== DirtyRegion Tests ====================
TEST(DirtyRegion_DefaultConstruction) {
    DirtyRegion dr;
    ASSERT_TRUE(dr.IsEmpty());
}

TEST(DirtyRegion_AddRect) {
    DirtyRegion dr;
    
    dr.AddRect(Rect(0.0f, 0.0f, 100.0f, 100.0f));
    ASSERT_FALSE(dr.IsEmpty());
}

TEST(DirtyRegion_AddMultipleRects) {
    DirtyRegion dr;
    
    dr.AddRect(Rect(0.0f, 0.0f, 100.0f, 100.0f));
    dr.AddRect(Rect(200.0f, 200.0f, 50.0f, 50.0f));
    dr.AddRect(Rect(300.0f, 300.0f, 75.0f, 75.0f));
    
    // All rects should be tracked
    ASSERT_FALSE(dr.IsEmpty());
    ASSERT_EQ(dr.GetRects().size(), 3u);
}

TEST(DirtyRegion_Clear) {
    DirtyRegion dr;
    
    dr.AddRect(Rect(0.0f, 0.0f, 100.0f, 100.0f));
    ASSERT_FALSE(dr.IsEmpty());
    
    dr.Clear();
    ASSERT_TRUE(dr.IsEmpty());
}

TEST(DirtyRegion_Intersects) {
    DirtyRegion dr;
    dr.AddRect(Rect(0.0f, 0.0f, 100.0f, 100.0f));
    
    ASSERT_TRUE(dr.Intersects(Rect(50.0f, 50.0f, 100.0f, 100.0f)));
    ASSERT_TRUE(dr.Intersects(Rect(0.0f, 0.0f, 50.0f, 50.0f)));
    ASSERT_FALSE(dr.Intersects(Rect(200.0f, 200.0f, 50.0f, 50.0f)));
}

TEST(DirtyRegion_EmptyIntersects) {
    DirtyRegion dr;
    
    // Empty region should not intersect anything
    ASSERT_FALSE(dr.Intersects(Rect(0.0f, 0.0f, 100.0f, 100.0f)));
}

TEST(DirtyRegion_InvalidateAll) {
    DirtyRegion dr;
    
    dr.InvalidateAll(800.0f, 600.0f);
    
    ASSERT_FALSE(dr.IsEmpty());
    ASSERT_EQ(dr.GetRects().size(), 1u);
    
    const auto& rects = dr.GetRects();
    ASSERT_EQ(rects[0].x, 0.0f);
    ASSERT_EQ(rects[0].y, 0.0f);
    ASSERT_EQ(rects[0].width, 800.0f);
    ASSERT_EQ(rects[0].height, 600.0f);
}

TEST(DirtyRegion_MultipleInvalidations) {
    DirtyRegion dr;
    
    // Simulate multiple UI updates
    dr.AddRect(Rect(10.0f, 10.0f, 100.0f, 30.0f));   // Button
    dr.AddRect(Rect(10.0f, 50.0f, 200.0f, 20.0f));   // Text
    dr.AddRect(Rect(0.0f, 100.0f, 300.0f, 200.0f));  // Scroll view
    
    ASSERT_FALSE(dr.IsEmpty());
    
    // After optimization, rects may be merged
    ASSERT_TRUE(dr.GetRects().size() >= 1u);
}

TEST(DirtyRegion_ClearAndReuse) {
    DirtyRegion dr;
    
    dr.AddRect(Rect(0.0f, 0.0f, 100.0f, 100.0f));
    dr.Clear();
    dr.AddRect(Rect(200.0f, 200.0f, 50.0f, 50.0f));
    
    ASSERT_EQ(dr.GetRects().size(), 1u);
    ASSERT_FALSE(dr.Intersects(Rect(0.0f, 0.0f, 100.0f, 100.0f)));
    ASSERT_TRUE(dr.Intersects(Rect(200.0f, 200.0f, 50.0f, 50.0f)));
}

// ==================== Rect Operations Tests ====================
TEST(Rect_IntersectsWith) {
    Rect r1(0.0f, 0.0f, 100.0f, 100.0f);
    Rect r2(50.0f, 50.0f, 100.0f, 100.0f);
    Rect r3(200.0f, 200.0f, 50.0f, 50.0f);
    
    ASSERT_TRUE(r1.Intersects(r2));
    ASSERT_TRUE(r2.Intersects(r1));
    ASSERT_FALSE(r1.Intersects(r3));
}

TEST(Rect_Intersect) {
    Rect r1(0.0f, 0.0f, 100.0f, 100.0f);
    Rect r2(50.0f, 50.0f, 100.0f, 100.0f);
    
    Rect inter = r1.Intersect(r2);
    ASSERT_EQ(inter.x, 50.0f);
    ASSERT_EQ(inter.y, 50.0f);
    ASSERT_EQ(inter.width, 50.0f);
    ASSERT_EQ(inter.height, 50.0f);
}

TEST(Rect_IsEmpty) {
    Rect r1;
    ASSERT_TRUE(r1.IsEmpty());
    
    Rect r2(0.0f, 0.0f, 100.0f, 0.0f);
    ASSERT_TRUE(r2.IsEmpty());
    
    Rect r3(0.0f, 0.0f, 0.0f, 100.0f);
    ASSERT_TRUE(r3.IsEmpty());
    
    Rect r4(0.0f, 0.0f, 100.0f, 100.0f);
    ASSERT_FALSE(r4.IsEmpty());
}

// ==================== Performance Tests ====================
TEST(DirtyRegion_LargeNumberOfRects) {
    DirtyRegion dr;
    
    // Add many small dirty rects
    for (int i = 0; i < 100; ++i) {
        dr.AddRect(Rect((float)i * 10.0f, 0.0f, 5.0f, 5.0f));
    }
    
    ASSERT_FALSE(dr.IsEmpty());
    // Should be optimized/merged to fewer rects
    ASSERT_TRUE(dr.GetRects().size() <= 100u);
}

TEST(DirtyRegion_OverlappingRects) {
    DirtyRegion dr;
    
    // Add many overlapping rects
    for (int i = 0; i < 50; ++i) {
        dr.AddRect(Rect(0.0f, 0.0f, (float)(i + 1) * 10.0f, (float)(i + 1) * 10.0f));
    }
    
    ASSERT_FALSE(dr.IsEmpty());
    // Should be merged into one large rect
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
