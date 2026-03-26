// Event System Tests - Delegate and Events
// Note: Using lambdas with captures to work around Delegate implementation limitations
#include "TestFramework.h"
#include "../core/Delegate.h"
#include "Button.h"
#include "Slider.h"
#include "CheckBox.h"
#include "TextBox.h"

using namespace luaui;
using namespace luaui::controls;

// ==================== Delegate Basic Tests ====================
TEST(Delegate_DefaultConstruction) {
    Delegate<int> delegate;
    
    ASSERT_TRUE(delegate.IsEmpty());
    ASSERT_EQ(delegate.Count(), 0u);
}

TEST(Delegate_AddHandler) {
    int callCount = 0;
    
    Delegate<int> delegate;
    
    // Use lambda with capture to avoid function pointer signature issues
    auto id = delegate.Add([&callCount](int value) {
        callCount += value;
    });
    
    ASSERT_NE(id, Delegate<int>::INVALID_ID);
    ASSERT_FALSE(delegate.IsEmpty());
    ASSERT_EQ(delegate.Count(), 1u);
}

TEST(Delegate_Invoke) {
    int sum = 0;
    
    Delegate<int, int> delegate;
    
    int localSum = 0;
    delegate.Add([&localSum](int a, int b) {
        localSum += a + b;
    });
    
    delegate.Invoke(5, 10);
    ASSERT_EQ(localSum, 15);
    
    delegate.Invoke(20, 30);
    ASSERT_EQ(localSum, 65);
}

TEST(Delegate_MultipleHandlers) {
    int value = 0;
    
    Delegate<int> delegate;
    
    int localValue = 0;
    delegate.Add([&localValue](int v) { localValue += v; });
    delegate.Add([&localValue](int v) { localValue += v * 2; });
    delegate.Add([&localValue](int v) { localValue += v * 3; });
    
    delegate.Invoke(10);
    ASSERT_EQ(localValue, 10 + 20 + 30);  // 60
}

TEST(Delegate_Remove) {
    int value = 0;
    
    Delegate<int> delegate;
    
    int localValue = 0;
    auto id1 = delegate.Add([&localValue](int v) { localValue += v; });
    auto id2 = delegate.Add([&localValue](int v) { localValue += v * 10; });
    
    delegate.Invoke(5);
    ASSERT_EQ(localValue, 5 + 50);  // 55
    
    delegate.Remove(id2);
    
    localValue = 0;
    delegate.Invoke(5);
    ASSERT_EQ(localValue, 5);  // Only first handler
}

TEST(Delegate_Clear) {
    Delegate<int> delegate;
    
    delegate.Add([](int) {});
    delegate.Add([](int) {});
    delegate.Add([](int) {});
    
    ASSERT_EQ(delegate.Count(), 3u);
    
    delegate.Clear();
    
    ASSERT_TRUE(delegate.IsEmpty());
    ASSERT_EQ(delegate.Count(), 0u);
}

// ==================== Delegate with Lambda Capture Tests ====================
TEST(Delegate_LambdaCapture) {
    int capturedValue = 100;
    
    Delegate<int> delegate;
    
    delegate.Add([&capturedValue](int v) {
        capturedValue += v;
    });
    
    delegate.Invoke(50);
    ASSERT_EQ(capturedValue, 150);
}

TEST(Delegate_ComplexLambda) {
    std::vector<int> results;
    
    Delegate<int> delegate;
    
    // Lambda that captures vector by reference
    delegate.Add([&results](int v) {
        results.push_back(v);
    });
    
    delegate.Invoke(1);
    delegate.Invoke(2);
    delegate.Invoke(3);
    
    ASSERT_EQ(results.size(), 3u);
    ASSERT_EQ(results[0], 1);
    ASSERT_EQ(results[1], 2);
    ASSERT_EQ(results[2], 3);
}

// ==================== Delegate Member Function Tests ====================
class TestEventHandler {
public:
    int callCount = 0;
    int lastValue = 0;
    
    void Handle(int value) {
        callCount++;
        lastValue = value;
    }
    
    void HandleTwoArgs(int a, int b) {
        callCount++;
        lastValue = a + b;
    }
};

TEST(Delegate_MemberFunction) {
    TestEventHandler handler;
    
    Delegate<int> delegate;
    auto id = delegate.Add(&handler, &TestEventHandler::Handle);
    
    delegate.Invoke(42);
    
    ASSERT_EQ(handler.callCount, 1);
    ASSERT_EQ(handler.lastValue, 42);
}

TEST(Delegate_MemberFunctionMultipleCalls) {
    TestEventHandler handler;
    
    Delegate<int> delegate;
    delegate.Add(&handler, &TestEventHandler::Handle);
    
    delegate.Invoke(10);
    delegate.Invoke(20);
    delegate.Invoke(30);
    
    ASSERT_EQ(handler.callCount, 3);
    ASSERT_EQ(handler.lastValue, 30);
}

// ==================== Delegate No Args Tests ====================
TEST(Delegate_NoArguments) {
    int count = 0;
    
    Delegate<> delegate;
    
    delegate.Add([&count]() { count++; });
    delegate.Add([&count]() { count += 10; });
    
    delegate.Invoke();
    ASSERT_EQ(count, 11);
}

// ==================== Delegate Edge Cases ====================
TEST(Delegate_InvokeOnEmpty) {
    Delegate<int> delegate;
    
    // Should not crash
    delegate.Invoke(42);
    
    ASSERT_TRUE(delegate.IsEmpty());
}

TEST(Delegate_RemoveInvalid) {
    Delegate<int> delegate;
    
    // Should not crash
    delegate.Remove(Delegate<int>::INVALID_ID);
    delegate.Remove(999);
    
    ASSERT_TRUE(delegate.IsEmpty());
}

TEST(Delegate_Reserve) {
    Delegate<int> delegate;
    delegate.Reserve(100);
    
    for (int i = 0; i < 50; ++i) {
        delegate.Add([](int) {});
    }
    
    ASSERT_EQ(delegate.Count(), 50u);
}

// ==================== Control Events Tests ====================
TEST(Button_ClickEvent) {
    auto button = std::make_shared<Button>();
    bool clicked = false;
    
    button->Click.Add([&clicked](Control*) {
        clicked = true;
    });
    
    // Simulate click
    button->Click.Invoke(button.get());
    
    ASSERT_TRUE(clicked);
}

TEST(Button_MultipleClickHandlers) {
    auto button = std::make_shared<Button>();
    int count = 0;
    
    button->Click.Add([&count](Control*) { count++; });
    button->Click.Add([&count](Control*) { count += 10; });
    
    button->Click.Invoke(button.get());
    
    ASSERT_EQ(count, 11);
}

TEST(Slider_ValueChangedEvent) {
    auto slider = std::make_shared<Slider>();
    double lastValue = -1.0;
    
    slider->ValueChanged.Add([&lastValue](Slider*, double value) {
        lastValue = value;
    });
    
    slider->SetValue(50.0);
    slider->ValueChanged.Invoke(slider.get(), 50.0);
    
    ASSERT_NEAR(lastValue, 50.0, 0.001);
}

TEST(CheckBox_CheckedEvent) {
    auto checkbox = std::make_shared<CheckBox>();
    bool eventFired = false;
    bool checkedState = false;
    
    checkbox->CheckedChanged.Add([&](CheckBox* cb, bool checked) {
        eventFired = true;
        checkedState = checked;
    });
    
    checkbox->SetIsChecked(true);
    checkbox->CheckedChanged.Invoke(checkbox.get(), true);
    
    ASSERT_TRUE(eventFired);
    ASSERT_TRUE(checkedState);
}

TEST(TextBox_TextChangedEvent) {
    auto textbox = std::make_shared<TextBox>();
    std::wstring lastText;
    
    textbox->TextChanged.Add([&lastText](TextBox*, const std::wstring& text) {
        lastText = text;
    });
    
    textbox->SetText(L"Hello");
    textbox->TextChanged.Invoke(textbox.get(), L"Hello");
    
    ASSERT_TRUE(lastText == L"Hello");
}

// ==================== Event Connection/Disconnection Tests ====================
TEST(Delegate_DisconnectAfterInvoke) {
    int value = 0;
    
    Delegate<int> delegate;
    
    auto id = delegate.Add([&value](int v) { value += v; });
    
    delegate.Invoke(10);
    ASSERT_EQ(value, 10);
    
    delegate.Remove(id);
    
    delegate.Invoke(20);
    ASSERT_EQ(value, 10);  // Unchanged after disconnect
}

TEST(Delegate_Reconnect) {
    int value = 0;
    
    Delegate<int> delegate;
    
    auto id = delegate.Add([&value](int v) { value += v; });
    
    delegate.Invoke(10);
    ASSERT_EQ(value, 10);
    
    delegate.Remove(id);
    delegate.Invoke(20);
    ASSERT_EQ(value, 10);
    
    // Reconnect new handler
    delegate.Add([&value](int v) { value += v * 2; });
    delegate.Invoke(10);
    ASSERT_EQ(value, 30);
}

// ==================== Performance Tests ====================
TEST(Delegate_ManyHandlers) {
    Delegate<int> delegate;
    int sum = 0;
    
    for (int i = 0; i < 100; ++i) {
        delegate.Add([&sum](int v) { sum += v; });
    }
    
    delegate.Invoke(1);
    ASSERT_EQ(sum, 100);
}

TEST(Delegate_FrequentAddRemove) {
    Delegate<int> delegate;
    
    for (int i = 0; i < 100; ++i) {
        auto id = delegate.Add([](int) {});
        delegate.Remove(id);
    }
    
    ASSERT_TRUE(delegate.IsEmpty());
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
