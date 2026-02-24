// Core Module - Delegate Tests (Simplified)
#include "TestFramework.h"
#include "Delegate.h"
#include <string>

using namespace luaui;

// ==================== Basic Delegate Tests ====================
TEST(Delegate_EmptyInvoke) {
    Delegate<> d;
    // Should not crash when invoking empty delegate
    d.Invoke();
    ASSERT_TRUE(true);
}

TEST(Delegate_LambdaNoCapture) {
    Delegate<> d;
    bool called = false;
    
    d.Add([&called]() { called = true; });
    d.Invoke();
    
    ASSERT_TRUE(called);
}

TEST(Delegate_MultipleHandlers) {
    Delegate<> d;
    int count = 0;
    
    d.Add([&count]() { count += 1; });
    d.Add([&count]() { count += 10; });
    d.Add([&count]() { count += 100; });
    
    d.Invoke();
    ASSERT_EQ(count, 111);
}

TEST(Delegate_RemoveHandler) {
    Delegate<> d;
    int count = 0;
    
    auto id1 = d.Add([&count]() { count += 1; });
    auto id2 = d.Add([&count]() { count += 10; });
    
    d.Invoke();
    ASSERT_EQ(count, 11);
    
    d.Remove(id1);
    d.Invoke();
    ASSERT_EQ(count, 21);  // Only second handler called again
}

TEST(Delegate_Clear) {
    Delegate<> d;
    int count = 0;
    
    d.Add([&count]() { count += 1; });
    d.Add([&count]() { count += 10; });
    
    d.Clear();
    d.Invoke();
    
    ASSERT_EQ(count, 0);  // No handlers called
}

TEST(Delegate_Count) {
    Delegate<> d;
    ASSERT_EQ(d.Count(), 0u);
    ASSERT_TRUE(d.IsEmpty());
    
    d.Add([]() {});
    ASSERT_EQ(d.Count(), 1u);
    ASSERT_FALSE(d.IsEmpty());
    
    auto id = d.Add([]() {});
    ASSERT_EQ(d.Count(), 2u);
    
    d.Remove(id);
    ASSERT_EQ(d.Count(), 1u);
}

TEST(Delegate_Reserve) {
    Delegate<> d;
    d.Reserve(100);
    ASSERT_TRUE(d.IsEmpty());
    ASSERT_EQ(d.Count(), 0u);
}

TEST(Delegate_InvalidRemove) {
    Delegate<> d;
    d.Remove(0);
    d.Remove(999);
    ASSERT_TRUE(true);
}

TEST(Delegate_WithParams) {
    Delegate<int, int> d;
    int sum = 0;
    
    d.Add([&sum](int a, int b) { sum = a + b; });
    d.Invoke(3, 4);
    
    ASSERT_EQ(sum, 7);
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
