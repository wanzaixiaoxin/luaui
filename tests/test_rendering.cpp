// Rendering Module Unit Tests
#include "TestFramework.h"
#include "luaui/rendering/Types.h"
#include <cmath>

using namespace luaui::rendering;

// ==================== Color Tests ====================
TEST(Color_Construction) {
    Color c1;
    ASSERT_EQ(c1.r, 0.0f);
    ASSERT_EQ(c1.g, 0.0f);
    ASSERT_EQ(c1.b, 0.0f);
    ASSERT_EQ(c1.a, 1.0f);
    
    Color c2(1.0f, 0.5f, 0.25f, 0.8f);
    ASSERT_EQ(c2.r, 1.0f);
    ASSERT_EQ(c2.g, 0.5f);
    ASSERT_EQ(c2.b, 0.25f);
    ASSERT_EQ(c2.a, 0.8f);
}

TEST(Color_FromRGBA) {
    Color c = Color::FromRGBA(255, 128, 64, 200);
    ASSERT_NEAR(c.r, 1.0f, 0.01f);
    ASSERT_NEAR(c.g, 0.5f, 0.01f);
    ASSERT_NEAR(c.b, 0.25f, 0.01f);
    ASSERT_NEAR(c.a, 0.78f, 0.01f);
}

TEST(Color_FromHex) {
    Color c1 = Color::FromHex(0xFF8040);
    ASSERT_NEAR(c1.r, 1.0f, 0.01f);
    ASSERT_NEAR(c1.g, 0.5f, 0.01f);
    ASSERT_NEAR(c1.b, 0.25f, 0.01f);
    ASSERT_EQ(c1.a, 1.0f);
    
    // AARRGGBB format (CC = 204 = 0.8 alpha)
    Color c2 = Color::FromHex(0xCCFF8040);
    ASSERT_NEAR(c2.r, 1.0f, 0.01f);
    ASSERT_NEAR(c2.g, 0.5f, 0.01f);
    ASSERT_NEAR(c2.b, 0.25f, 0.01f);
    ASSERT_NEAR(c2.a, 0.8f, 0.01f);
}

TEST(Color_Lerp) {
    Color c1(1.0f, 0.0f, 0.0f, 1.0f);  // Red
    Color c2(0.0f, 0.0f, 1.0f, 1.0f);  // Blue
    
    Color mid = c1.Lerp(c2, 0.5f);
    ASSERT_NEAR(mid.r, 0.5f, 0.01f);
    ASSERT_NEAR(mid.g, 0.0f, 0.01f);
    ASSERT_NEAR(mid.b, 0.5f, 0.01f);
    ASSERT_EQ(mid.a, 1.0f);
}

TEST(Color_Premultiply) {
    Color c(1.0f, 0.5f, 0.25f, 0.5f);
    Color p = c.Premultiply();
    ASSERT_NEAR(p.r, 0.5f, 0.01f);
    ASSERT_NEAR(p.g, 0.25f, 0.01f);
    ASSERT_NEAR(p.b, 0.125f, 0.01f);
    ASSERT_EQ(p.a, 0.5f);
}

TEST(Color_PredefinedColors) {
    Color red = Color::Red();
    ASSERT_EQ(red.r, 1.0f);
    ASSERT_EQ(red.g, 0.0f);
    ASSERT_EQ(red.b, 0.0f);
    ASSERT_EQ(red.a, 1.0f);
    
    Color green = Color::Green();
    ASSERT_EQ(green.g, 1.0f);
    
    Color blue = Color::Blue();
    ASSERT_EQ(blue.b, 1.0f);
    
    Color white = Color::White();
    ASSERT_EQ(white.r, 1.0f);
    ASSERT_EQ(white.g, 1.0f);
    ASSERT_EQ(white.b, 1.0f);
    
    Color black = Color::Black();
    ASSERT_EQ(black.r, 0.0f);
    ASSERT_EQ(black.g, 0.0f);
    ASSERT_EQ(black.b, 0.0f);
    
    Color transparent = Color::Transparent();
    ASSERT_EQ(transparent.a, 0.0f);
}

TEST(Color_Clamping) {
    // Values should be clamped to [0, 1]
    Color c(-0.5f, 1.5f, 2.0f, -1.0f);
    ASSERT_EQ(c.r, 0.0f);
    ASSERT_EQ(c.g, 1.0f);
    ASSERT_EQ(c.b, 1.0f);
    ASSERT_EQ(c.a, 0.0f);
}

// ==================== Point Tests ====================
TEST(Point_Construction) {
    Point p1;
    ASSERT_EQ(p1.x, 0.0f);
    ASSERT_EQ(p1.y, 0.0f);
    
    Point p2(10.0f, 20.0f);
    ASSERT_EQ(p2.x, 10.0f);
    ASSERT_EQ(p2.y, 20.0f);
}

TEST(Point_Arithmetic) {
    Point p1(10.0f, 20.0f);
    Point p2(5.0f, 3.0f);
    
    Point sum = p1 + p2;
    ASSERT_EQ(sum.x, 15.0f);
    ASSERT_EQ(sum.y, 23.0f);
    
    Point diff = p1 - p2;
    ASSERT_EQ(diff.x, 5.0f);
    ASSERT_EQ(diff.y, 17.0f);
    
    Point scaled = p1 * 2.0f;
    ASSERT_EQ(scaled.x, 20.0f);
    ASSERT_EQ(scaled.y, 40.0f);
}

// ==================== Rect Tests ====================
TEST(Rect_Construction) {
    Rect r1;
    ASSERT_EQ(r1.x, 0.0f);
    ASSERT_EQ(r1.y, 0.0f);
    ASSERT_EQ(r1.width, 0.0f);
    ASSERT_EQ(r1.height, 0.0f);
    
    Rect r2(10.0f, 20.0f, 100.0f, 200.0f);
    ASSERT_EQ(r2.x, 10.0f);
    ASSERT_EQ(r2.y, 20.0f);
    ASSERT_EQ(r2.width, 100.0f);
    ASSERT_EQ(r2.height, 200.0f);
    
    ASSERT_EQ(r2.Left(), 10.0f);
    ASSERT_EQ(r2.Top(), 20.0f);
    ASSERT_EQ(r2.Right(), 110.0f);
    ASSERT_EQ(r2.Bottom(), 220.0f);
}

TEST(Rect_PositionAndSize) {
    Rect r(10.0f, 20.0f, 100.0f, 200.0f);
    
    Point pos = r.Position();
    ASSERT_EQ(pos.x, 10.0f);
    ASSERT_EQ(pos.y, 20.0f);
    
    Size size = r.GetSize();
    ASSERT_EQ(size.width, 100.0f);
    ASSERT_EQ(size.height, 200.0f);
}

TEST(Rect_Contains) {
    Rect r(10.0f, 10.0f, 100.0f, 100.0f);
    
    ASSERT_TRUE(r.Contains(Point(50.0f, 50.0f)));   // Inside
    ASSERT_TRUE(r.Contains(Point(10.0f, 10.0f)));   // On edge
    ASSERT_FALSE(r.Contains(Point(5.0f, 5.0f)));    // Outside
    ASSERT_FALSE(r.Contains(Point(200.0f, 50.0f))); // Outside right
}

TEST(Rect_Intersects) {
    Rect r1(0.0f, 0.0f, 100.0f, 100.0f);
    Rect r2(50.0f, 50.0f, 100.0f, 100.0f);
    
    ASSERT_TRUE(r1.Intersects(r2));
    
    Rect r3(200.0f, 200.0f, 50.0f, 50.0f);
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

TEST(Rect_FromPointAndSize) {
    Point p(10.0f, 20.0f);
    Size s(100.0f, 200.0f);
    Rect r(p, s);
    
    ASSERT_EQ(r.x, 10.0f);
    ASSERT_EQ(r.y, 20.0f);
    ASSERT_EQ(r.width, 100.0f);
    ASSERT_EQ(r.height, 200.0f);
}

// ==================== Transform Tests ====================
TEST(Transform_Identity) {
    Transform t = Transform::Identity();
    Point p(10.0f, 20.0f);
    Point r = t.TransformPoint(p);
    ASSERT_EQ(r.x, 10.0f);
    ASSERT_EQ(r.y, 20.0f);
}

TEST(Transform_Translation) {
    Transform t = Transform::Translation(5.0f, 10.0f);
    Point p(10.0f, 20.0f);
    Point r = t.TransformPoint(p);
    ASSERT_EQ(r.x, 15.0f);
    ASSERT_EQ(r.y, 30.0f);
}

TEST(Transform_Scale) {
    Transform t = Transform::Scale(2.0f, 3.0f);
    Point p(10.0f, 20.0f);
    Point r = t.TransformPoint(p);
    ASSERT_EQ(r.x, 20.0f);
    ASSERT_EQ(r.y, 60.0f);
}

TEST(Transform_Rotation) {
    Transform t = Transform::Rotation(90.0f); // 90 degrees
    Point p(1.0f, 0.0f);
    Point r = t.TransformPoint(p);
    ASSERT_NEAR(r.x, 0.0f, 0.0001f);
    ASSERT_NEAR(r.y, 1.0f, 0.0001f);
}

TEST(Transform_Multiply) {
    // Combined = Scale * Translate
    // First scale, then translate
    Transform scale = Transform::Scale(2.0f, 1.0f);
    Transform translate = Transform::Translation(10.0f, 0.0f);
    Transform combined = scale * translate;
    
    Point p(10.0f, 5.0f);
    Point r = combined.TransformPoint(p);
    // Scale first: (10, 5) -> (20, 5)
    // Then translate: (20, 5) -> (30, 5)
    ASSERT_EQ(r.x, 30.0f);
    ASSERT_EQ(r.y, 5.0f);
}

TEST(Transform_GetElements) {
    Transform t = Transform::Identity();
    const float* m = t.GetElements();
    ASSERT_EQ(m[0], 1.0f);
    ASSERT_EQ(m[1], 0.0f);
    ASSERT_EQ(m[2], 0.0f);
    ASSERT_EQ(m[3], 1.0f);
    ASSERT_EQ(m[4], 0.0f);
    ASSERT_EQ(m[5], 0.0f);
}

// ==================== Size Tests ====================
TEST(Size_Construction) {
    Size s1;
    ASSERT_EQ(s1.width, 0.0f);
    ASSERT_EQ(s1.height, 0.0f);
    
    Size s2(800.0f, 600.0f);
    ASSERT_EQ(s2.width, 800.0f);
    ASSERT_EQ(s2.height, 600.0f);
}

TEST(Size_IsEmpty) {
    Size s1(0.0f, 100.0f);
    ASSERT_TRUE(s1.IsEmpty());
    
    Size s2(100.0f, 0.0f);
    ASSERT_TRUE(s2.IsEmpty());
    
    Size s3(100.0f, 100.0f);
    ASSERT_FALSE(s3.IsEmpty());
}

TEST(Size_Scale) {
    Size s(100.0f, 200.0f);
    Size scaled = s * 2.0f;
    ASSERT_EQ(scaled.width, 200.0f);
    ASSERT_EQ(scaled.height, 400.0f);
}

// ==================== CornerRadius Tests ====================
TEST(CornerRadius_Construction) {
    CornerRadius cr1(10.0f);
    ASSERT_EQ(cr1.topLeft, 10.0f);
    ASSERT_EQ(cr1.topRight, 10.0f);
    ASSERT_EQ(cr1.bottomRight, 10.0f);
    ASSERT_EQ(cr1.bottomLeft, 10.0f);
    
    CornerRadius cr2(5.0f, 10.0f, 15.0f, 20.0f);
    ASSERT_EQ(cr2.topLeft, 5.0f);
    ASSERT_EQ(cr2.topRight, 10.0f);
    ASSERT_EQ(cr2.bottomRight, 15.0f);
    ASSERT_EQ(cr2.bottomLeft, 20.0f);
}

TEST(CornerRadius_Default) {
    CornerRadius cr;
    ASSERT_EQ(cr.topLeft, 0.0f);
    ASSERT_EQ(cr.topRight, 0.0f);
    ASSERT_EQ(cr.bottomRight, 0.0f);
    ASSERT_EQ(cr.bottomLeft, 0.0f);
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
