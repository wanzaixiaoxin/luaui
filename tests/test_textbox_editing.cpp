#include "TestFramework.h"
#include "TextBox.h"
#include <windows.h>

using namespace luaui;
using namespace luaui::controls;

namespace {

class TestableTextBox : public TextBox {
public:
    using TextBox::OnChar;
    using TextBox::OnKeyDown;
    using TextBox::OnMouseDown;
    using TextBox::OnMouseMove;
    using TextBox::OnMouseUp;
};

void SendKey(TestableTextBox& textBox, int keyCode, bool control = false, bool shift = false) {
    KeyEventArgs args;
    args.keyCode = keyCode;
    args.Control = control;
    args.Shift = shift;
    textBox.OnKeyDown(args);
}

float TextXForPosition(float controlX, int position) {
    return controlX + 1.0f + 6.0f + (static_cast<float>(position) * 14.0f * 0.6f);
}

} // namespace

TEST(TextBox_KeyboardSelectionAndReplace) {
    TestableTextBox textBox;
    textBox.SetText(L"hello world");
    textBox.SetPlaceholder(L"placeholder");

    SendKey(textBox, 'A', true, false);
    ASSERT_TRUE(textBox.HasSelection());
    ASSERT_TRUE(textBox.GetSelectedText() == L"hello world");
    ASSERT_EQ(textBox.GetMaxLength(), 0);
    ASSERT_TRUE(textBox.GetPlaceholder() == L"placeholder");

    textBox.OnChar(L'X');

    ASSERT_TRUE(textBox.GetText() == L"X");
    ASSERT_FALSE(textBox.HasSelection());
    ASSERT_EQ(textBox.GetCaretPosition(), 1);
}

TEST(TextBox_UndoRedoRoundTrip) {
    TestableTextBox textBox;

    textBox.OnChar(L'a');
    textBox.OnChar(L'b');
    textBox.OnChar(L'c');

    ASSERT_TRUE(textBox.GetText() == L"abc");
    ASSERT_TRUE(textBox.CanUndo());

    textBox.Undo();
    ASSERT_TRUE(textBox.GetText() == L"ab");

    textBox.Undo();
    ASSERT_TRUE(textBox.GetText() == L"a");

    textBox.Redo();
    ASSERT_TRUE(textBox.GetText() == L"ab");

    textBox.Redo();
    ASSERT_TRUE(textBox.GetText() == L"abc");
}

TEST(TextBox_WordNavigationAndDeletion) {
    TestableTextBox textBox;
    textBox.SetText(L"alpha beta gamma");
    textBox.SetCaretPosition(0);

    SendKey(textBox, VK_RIGHT, true, false);
    ASSERT_EQ(textBox.GetCaretPosition(), 6);

    SendKey(textBox, VK_RIGHT, true, false);
    ASSERT_EQ(textBox.GetCaretPosition(), 11);

    SendKey(textBox, VK_LEFT, true, false);
    ASSERT_EQ(textBox.GetCaretPosition(), 6);

    SendKey(textBox, VK_BACK, true, false);
    ASSERT_TRUE(textBox.GetText() == L"beta gamma");
    ASSERT_EQ(textBox.GetCaretPosition(), 0);
}

TEST(TextBox_MouseHitTestUsesAbsoluteOffset) {
    TestableTextBox textBox;
    textBox.SetText(L"abcdef");
    textBox.GetRender()->GetRenderRect() = rendering::Rect(120.0f, 40.0f, 160.0f, 28.0f);

    MouseEventArgs args;
    args.x = TextXForPosition(120.0f, 3);
    args.y = 52.0f;
    args.button = 0;
    textBox.OnMouseDown(args);

    ASSERT_EQ(textBox.GetCaretPosition(), 3);
}

TEST(TextBox_MouseDragCreatesSelection) {
    TestableTextBox textBox;
    textBox.SetText(L"abcdef");
    textBox.GetRender()->GetRenderRect() = rendering::Rect(80.0f, 20.0f, 160.0f, 28.0f);

    MouseEventArgs down;
    down.x = TextXForPosition(80.0f, 1);
    down.y = 32.0f;
    down.button = 0;
    textBox.OnMouseDown(down);

    MouseEventArgs move;
    move.x = TextXForPosition(80.0f, 4);
    move.y = 32.0f;
    move.button = 0;
    textBox.OnMouseMove(move);

    MouseEventArgs up;
    up.x = TextXForPosition(80.0f, 4);
    up.y = 32.0f;
    up.button = 0;
    textBox.OnMouseUp(up);

    ASSERT_TRUE(textBox.HasSelection());
    ASSERT_TRUE(textBox.GetSelectedText() == L"bcd");
}

TEST(TextBox_ReadOnlyBlocksMutationButKeepsSelectionAPIs) {
    TestableTextBox textBox;
    textBox.SetText(L"hello");
    textBox.Select(1, 3);
    textBox.SetIsReadOnly(true);

    SendKey(textBox, VK_BACK, false, false);
    ASSERT_TRUE(textBox.GetText() == L"hello");
    ASSERT_TRUE(textBox.GetSelectedText() == L"ell");

    textBox.OnChar(L'X');
    ASSERT_TRUE(textBox.GetText() == L"hello");
}

TEST(TextBox_MaxLengthAppliesToReplacement) {
    TestableTextBox textBox;
    textBox.SetMaxLength(5);
    textBox.SetText(L"hello");
    textBox.SelectAll();

    textBox.OnChar(L'w');
    textBox.OnChar(L'o');
    textBox.OnChar(L'r');
    textBox.OnChar(L'l');
    textBox.OnChar(L'd');
    textBox.OnChar(L'!');

    ASSERT_TRUE(textBox.GetText() == L"world");
    ASSERT_EQ(textBox.GetCaretPosition(), 5);
}

int main() {
    return RUN_ALL_TESTS();
}
