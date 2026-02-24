// Core Module - Delegate Tests
#include "TestFramework.h"
#include "Delegate.h"
#include <string>

using namespace luaui;

// Static counters for testing
static int staticCallCount = 0;
static void StaticHandler() {
    staticCallCount++;
}

static int staticIntValue = 0;
static void StaticHandlerWithInt(int val) {
    staticIntValue = val;
}

// ==================== Basic Delegate Tests ====================
TEST(Delegate_EmptyInvoke) {
    Delegate<> d;
    // Should not crash when invoking empty delegate
    d.Invoke();
    ASSERT_TRUE(true);
}

TEST(Delegate_StaticFunction) {
    Delegate<> d;
    staticCallCount = 0;
    
    d.Add(&StaticHandler);
    ASSERT_EQ(d.Count(), 1u);
    
    d.Invoke();
    ASSERT_EQ(staticCallCount, 1);
    
    d.Invoke();
    ASSERT_EQ(staticCallCount, 2);
}

TEST(Delegate_MultipleStaticHandlers) {
    Delegate<int> d;
    staticIntValue = 0;
    
    d.Add(&StaticHandlerWithInt);
    d.Invoke(42);
    
    ASSERT_EQ(staticIntValue, 42);
}

TEST(Delegate_RemoveHandler) {
    Delegate<> d;
    staticCallCount = 0;
    
    auto id1 = d.Add(&StaticHandler);
    d.Invoke();
    ASSERT_EQ(staticCallCount, 1);
    
    d.Remove(id1);
    d.Invoke();
    // Should not increment after removal
    ASSERT_EQ(staticCallCount, 1);
}

TEST(Delegate_Clear) {
    Delegate<> d;
    staticCallCount = 0;
    
    d.Add(&StaticHandler);
    d.Add(&StaticHandler);
    
    ASSERT_EQ(d.Count(), 2u);
    
    d.Clear();
    d.Invoke();
    
    ASSERT_EQ(staticCallCount, 0);  // No handlers called after clear
    ASSERT_EQ(d.Count(), 0u);
}

TEST(Delegate_Count) {
    Delegate<> d;
    ASSERT_EQ(d.Count(), 0u);
    ASSERT_TRUE(d.IsEmpty());
    
    d.Add(&StaticHandler);
    ASSERT_EQ(d.Count(), 1u);
    ASSERT_FALSE(d.IsEmpty());
    
    auto id = d.Add(&StaticHandler);
    ASSERT_EQ(d.Count(), 2u);
    
    d.Remove(id);
    ASSERT_EQ(d.Count(), 1u);
}

TEST(Delegate_Reserve) {
    Delegate<> d;
    d.Reserve(100);
    // Should not crash and should have reserved capacity
    ASSERT_TRUE(d.IsEmpty());
    ASSERT_EQ(d.Count(), 0u);
}

TEST(Delegate_InvalidRemove) {
    Delegate<> d;
    // Remove invalid ID should not crash
    d.Remove(0);
    d.Remove(999);
    ASSERT_TRUE(true);
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
