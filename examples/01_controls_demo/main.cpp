// LuaUI Controls Showcase - 分页展示所有控件
// 使用 TabControl 分页，避免控件遮挡

#include "Controls.h"
#include "Panel.h"
#include "Window.h"
#include "IRenderContext.h"
#include "Logger.h"
#include "Types.h"
#include "Shapes.h"
#include <windows.h>
#include <string>
#include <sstream>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;
using namespace luaui::utils;

class ControlsShowcaseWindow : public Window {
public:
    ControlsShowcaseWindow() = default;

protected:
    void OnLoaded() override;

private:
    // 创建各个分页内容
    std::shared_ptr<Panel> CreateBasicTab();
    std::shared_ptr<Panel> CreateInputTab();
    std::shared_ptr<Panel> CreateSelectionTab();
    std::shared_ptr<Panel> CreateDataTab();
    std::shared_ptr<Panel> CreateGraphicsTab();
    
    // 辅助方法
    std::shared_ptr<TextBlock> CreateTitle(const std::wstring& text);
    std::shared_ptr<TextBlock> CreateSubtitle(const std::wstring& text);
    std::shared_ptr<Panel> CreateCard(const std::wstring& title, const std::wstring& desc);
    std::shared_ptr<StackPanel> CreateRow();
    
    void UpdateStatus(const std::wstring& msg);
    
    // 控件引用
    std::shared_ptr<TextBlock> m_statusText;
    std::shared_ptr<ProgressBar> m_progressBar;
    std::shared_ptr<Slider> m_slider;
    std::shared_ptr<TextBlock> m_sliderValue;
    std::shared_ptr<TextBlock> m_clickResult;
    std::shared_ptr<ListBox> m_listBox;
};

// ========== 辅助方法 ==========

std::shared_ptr<TextBlock> ControlsShowcaseWindow::CreateTitle(const std::wstring& text) {
    auto t = std::make_shared<TextBlock>();
    t->SetText(text);
    t->SetFontSize(22);
    t->SetForeground(Color::FromHex(0x1976D2));
    t->SetFontWeight(FontWeight::Bold);
    return t;
}

std::shared_ptr<TextBlock> ControlsShowcaseWindow::CreateSubtitle(const std::wstring& text) {
    auto s = std::make_shared<TextBlock>();
    s->SetText(text);
    s->SetFontSize(13);
    s->SetForeground(Color::FromHex(0x757575));
    return s;
}

std::shared_ptr<StackPanel> ControlsShowcaseWindow::CreateRow() {
    auto row = std::make_shared<StackPanel>();
    row->SetOrientation(StackPanel::Orientation::Horizontal);
    row->SetSpacing(12);
    return row;
}

std::shared_ptr<Panel> ControlsShowcaseWindow::CreateCard(const std::wstring& title, const std::wstring& desc) {
    auto card = std::make_shared<StackPanel>();
    card->SetOrientation(StackPanel::Orientation::Vertical);
    card->SetSpacing(8);
    
    auto titleBlock = std::make_shared<TextBlock>();
    titleBlock->SetText(title);
    titleBlock->SetFontSize(14);
    titleBlock->SetForeground(Color::FromHex(0x424242));
    titleBlock->SetFontWeight(FontWeight::SemiBold);
    card->AddChild(titleBlock);
    
    if (!desc.empty()) {
        auto descBlock = std::make_shared<TextBlock>();
        descBlock->SetText(desc);
        descBlock->SetFontSize(11);
        descBlock->SetForeground(Color::FromHex(0x9E9E9E));
        card->AddChild(descBlock);
    }
    
    return card;
}

void ControlsShowcaseWindow::UpdateStatus(const std::wstring& msg) {
    if (m_statusText) m_statusText->SetText(msg);
}

// ========== Basic 页 ==========
std::shared_ptr<Panel> ControlsShowcaseWindow::CreateBasicTab() {
    auto root = std::make_shared<StackPanel>();
    root->SetOrientation(StackPanel::Orientation::Vertical);
    root->SetSpacing(20);
    
    // 按钮区域
    auto btnCard = CreateCard(L"Buttons", L"Various button styles and states");
    
    auto btnRow1 = CreateRow();
    
    auto btn1 = std::make_shared<Button>();
    btn1->SetText(L"Default");
    btn1->Click.Add([this](auto*) { UpdateStatus(L"Default button clicked"); });
    btnRow1->AddChild(btn1);
    
    auto btn2 = std::make_shared<Button>();
    btn2->SetText(L"Primary");
    btn2->SetStateColors(Color::FromHex(0x2196F3), Color::FromHex(0x1976D2), Color::FromHex(0x0D47A1));
    btn2->Click.Add([this](auto*) { UpdateStatus(L"Primary button clicked"); });
    btnRow1->AddChild(btn2);
    
    auto btn3 = std::make_shared<Button>();
    btn3->SetText(L"Success");
    btn3->SetStateColors(Color::FromHex(0x4CAF50), Color::FromHex(0x388E3C), Color::FromHex(0x1B5E20));
    btn3->Click.Add([this](auto*) { UpdateStatus(L"Success button clicked"); });
    btnRow1->AddChild(btn3);
    
    auto btn4 = std::make_shared<Button>();
    btn4->SetText(L"Danger");
    btn4->SetStateColors(Color::FromHex(0xF44336), Color::FromHex(0xD32F2F), Color::FromHex(0xB71C1C));
    btn4->Click.Add([this](auto*) { UpdateStatus(L"Danger button clicked"); });
    btnRow1->AddChild(btn4);
    
    btnCard->AddChild(btnRow1);
    root->AddChild(btnCard);
    
    // 文本样式区域
    auto textCard = CreateCard(L"Typography", L"Text formatting and styling options");
    
    auto textPanel = std::make_shared<StackPanel>();
    textPanel->SetOrientation(StackPanel::Orientation::Vertical);
    textPanel->SetSpacing(6);
    
    auto t1 = std::make_shared<TextBlock>(); t1->SetText(L"Normal text (Regular)"); textPanel->AddChild(t1);
    
    auto t2 = std::make_shared<TextBlock>();
    t2->SetText(L"Bold text (SemiBold)");
    t2->SetFontWeight(FontWeight::SemiBold);
    textPanel->AddChild(t2);
    
    auto t3 = std::make_shared<TextBlock>();
    t3->SetText(L"Italic text (Oblique)");
    t3->SetFontStyle(FontStyle::Italic);
    t3->SetForeground(Color::FromHex(0x666666));
    textPanel->AddChild(t3);
    
    auto t4 = std::make_shared<TextBlock>();
    t4->SetText(L"Large colored heading");
    t4->SetFontSize(18);
    t4->SetForeground(Color::FromHex(0xE91E63));
    t4->SetFontWeight(FontWeight::Bold);
    textPanel->AddChild(t4);
    
    auto t5 = std::make_shared<TextBlock>();
    t5->SetText(L"Small secondary text");
    t5->SetFontSize(10);
    t5->SetForeground(Color::FromHex(0x9E9E9E));
    textPanel->AddChild(t5);
    
    textCard->AddChild(textPanel);
    root->AddChild(textCard);
    
    return root;
}

// ========== Input 页 ==========
std::shared_ptr<Panel> ControlsShowcaseWindow::CreateInputTab() {
    auto root = std::make_shared<StackPanel>();
    root->SetOrientation(StackPanel::Orientation::Vertical);
    root->SetSpacing(20);
    
    // TextBox 区域
    auto textCard = CreateCard(L"Text Input", L"Single-line and multi-line text input");
    
    auto lbl1 = std::make_shared<TextBlock>();
    lbl1->SetText(L"Username:");
    lbl1->SetFontSize(12);
    lbl1->SetForeground(Color::FromHex(0x616161));
    textCard->AddChild(lbl1);
    
    auto textBox = std::make_shared<TextBox>();
    if (auto* l = textBox->GetLayout()) {
        l->SetWidth(350);
        l->SetHeight(32);
    }
    textCard->AddChild(textBox);
    
    auto lbl2 = std::make_shared<TextBlock>();
    lbl2->SetText(L"Description:");
    lbl2->SetFontSize(12);
    lbl2->SetForeground(Color::FromHex(0x616161));
    textCard->AddChild(lbl2);
    
    auto descBox = std::make_shared<TextBox>();
    if (auto* l = descBox->GetLayout()) {
        l->SetWidth(350);
        l->SetHeight(60);
    }
    textCard->AddChild(descBox);
    
    root->AddChild(textCard);
    
    // Slider & ProgressBar 区域
    auto sliderCard = CreateCard(L"Slider & ProgressBar", L"Interactive value selection and progress display");
    
    auto lbl3 = std::make_shared<TextBlock>();
    lbl3->SetText(L"Volume Control:");
    lbl3->SetFontSize(12);
    lbl3->SetForeground(Color::FromHex(0x616161));
    sliderCard->AddChild(lbl3);
    
    m_sliderValue = std::make_shared<TextBlock>();
    m_sliderValue->SetText(L"50%");
    m_sliderValue->SetFontWeight(FontWeight::SemiBold);
    sliderCard->AddChild(m_sliderValue);
    
    m_slider = std::make_shared<Slider>();
    m_slider->SetValue(50);
    m_slider->SetMinimum(0);
    m_slider->SetMaximum(100);
    if (auto* l = m_slider->GetLayout()) {
        l->SetWidth(400);
    }
    m_slider->ValueChanged.Add([this](auto*, double v) {
        std::wstringstream ss;
        ss << (int)v << L"%";
        if (m_sliderValue) m_sliderValue->SetText(ss.str());
        if (m_progressBar) m_progressBar->SetValue(v);
    });
    sliderCard->AddChild(m_slider);
    
    auto lbl4 = std::make_shared<TextBlock>();
    lbl4->SetText(L"Download Progress:");
    lbl4->SetFontSize(12);
    lbl4->SetForeground(Color::FromHex(0x616161));
    sliderCard->AddChild(lbl4);
    
    m_progressBar = std::make_shared<ProgressBar>();
    m_progressBar->SetValue(50);
    if (auto* l = m_progressBar->GetLayout()) {
        l->SetWidth(400);
        l->SetHeight(8);
    }
    sliderCard->AddChild(m_progressBar);
    
    root->AddChild(sliderCard);
    
    return root;
}

// ========== Selection 页 ==========
std::shared_ptr<Panel> ControlsShowcaseWindow::CreateSelectionTab() {
    auto root = std::make_shared<StackPanel>();
    root->SetOrientation(StackPanel::Orientation::Vertical);
    root->SetSpacing(20);
    
    // CheckBox 区域
    auto checkCard = CreateCard(L"CheckBoxes", L"Multiple selection options");
    
    auto chk1 = std::make_shared<CheckBox>();
    chk1->SetText(L"Enable notifications");
    chk1->SetIsChecked(true);
    chk1->CheckedChanged.Add([this](auto*, bool c) {
        UpdateStatus(c ? L"Notifications enabled" : L"Notifications disabled");
    });
    checkCard->AddChild(chk1);
    
    auto chk2 = std::make_shared<CheckBox>();
    chk2->SetText(L"Auto-save changes");
    chk2->CheckedChanged.Add([this](auto*, bool c) {
        UpdateStatus(c ? L"Auto-save enabled" : L"Auto-save disabled");
    });
    checkCard->AddChild(chk2);
    
    auto chk3 = std::make_shared<CheckBox>();
    chk3->SetText(L"Dark mode (experimental)");
    chk3->CheckedChanged.Add([this](auto*, bool c) {
        UpdateStatus(c ? L"Dark mode selected" : L"Light mode selected");
    });
    checkCard->AddChild(chk3);
    
    auto chk4 = std::make_shared<CheckBox>();
    chk4->SetText(L"Send analytics data");
    chk4->SetIsChecked(true);
    checkCard->AddChild(chk4);
    
    root->AddChild(checkCard);
    
    // ListBox 区域
    auto listCard = CreateCard(L"ListBox", L"Single selection from a list of items");
    
    m_listBox = std::make_shared<ListBox>();
    m_listBox->AddItem(L"Windows 11");
    m_listBox->AddItem(L"Windows 10");
    m_listBox->AddItem(L"Ubuntu 22.04 LTS");
    m_listBox->AddItem(L"macOS Sonoma");
    m_listBox->AddItem(L"Fedora 39");
    m_listBox->AddItem(L"Debian 12");
    if (auto* l = m_listBox->GetLayout()) {
        l->SetWidth(350);
        l->SetHeight(140);
    }
    listCard->AddChild(m_listBox);
    
    auto selectedText = std::make_shared<TextBlock>();
    selectedText->SetText(L"Selected: None");
    selectedText->SetFontSize(11);
    selectedText->SetForeground(Color::FromHex(0x757575));
    listCard->AddChild(selectedText);
    
    root->AddChild(listCard);
    
    return root;
}

// ========== Data 页 ==========
std::shared_ptr<Panel> ControlsShowcaseWindow::CreateDataTab() {
    auto root = std::make_shared<StackPanel>();
    root->SetOrientation(StackPanel::Orientation::Vertical);
    root->SetSpacing(20);
    
    // DataGrid 区域
    auto gridCard = CreateCard(L"DataGrid", L"Tabular data display with multiple columns");
    
    auto dataGrid = std::make_shared<DataGrid>();
    
    auto col1 = std::make_shared<DataGridColumn>(L"Product");
    col1->SetWidth(L"150");
    dataGrid->AddColumn(col1);
    
    auto col2 = std::make_shared<DataGridColumn>(L"Category");
    col2->SetWidth(L"100");
    dataGrid->AddColumn(col2);
    
    auto col3 = std::make_shared<DataGridColumn>(L"Price");
    col3->SetWidth(L"80");
    dataGrid->AddColumn(col3);
    
    auto col4 = std::make_shared<DataGridColumn>(L"Stock");
    col4->SetWidth(L"80");
    dataGrid->AddColumn(col4);
    
    // 添加数据行
    auto row1 = std::make_shared<DataGridRow>();
    auto c11 = std::make_shared<DataGridCell>(); c11->SetText(L"Wireless Mouse"); row1->AddCell(c11);
    auto c12 = std::make_shared<DataGridCell>(); c12->SetText(L"Electronics"); row1->AddCell(c12);
    auto c13 = std::make_shared<DataGridCell>(); c13->SetText(L"$29.99"); row1->AddCell(c13);
    auto c14 = std::make_shared<DataGridCell>(); c14->SetText(L"156"); row1->AddCell(c14);
    dataGrid->AddRow(row1);
    
    auto row2 = std::make_shared<DataGridRow>();
    auto c21 = std::make_shared<DataGridCell>(); c21->SetText(L"USB-C Cable"); row2->AddCell(c21);
    auto c22 = std::make_shared<DataGridCell>(); c22->SetText(L"Accessories"); row2->AddCell(c22);
    auto c23 = std::make_shared<DataGridCell>(); c23->SetText(L"$12.50"); row2->AddCell(c23);
    auto c24 = std::make_shared<DataGridCell>(); c24->SetText(L"423"); row2->AddCell(c24);
    dataGrid->AddRow(row2);
    
    auto row3 = std::make_shared<DataGridRow>();
    auto c31 = std::make_shared<DataGridCell>(); c31->SetText(L"Mechanical Keyboard"); row3->AddCell(c31);
    auto c32 = std::make_shared<DataGridCell>(); c32->SetText(L"Electronics"); row3->AddCell(c32);
    auto c33 = std::make_shared<DataGridCell>(); c33->SetText(L"$89.99"); row3->AddCell(c33);
    auto c34 = std::make_shared<DataGridCell>(); c34->SetText(L"67"); row3->AddCell(c34);
    dataGrid->AddRow(row3);
    
    auto row4 = std::make_shared<DataGridRow>();
    auto c41 = std::make_shared<DataGridCell>(); c41->SetText(L"Webcam 4K"); row4->AddCell(c41);
    auto c42 = std::make_shared<DataGridCell>(); c42->SetText(L"Electronics"); row4->AddCell(c42);
    auto c43 = std::make_shared<DataGridCell>(); c43->SetText(L"$129.00"); row4->AddCell(c43);
    auto c44 = std::make_shared<DataGridCell>(); c44->SetText(L"23"); row4->AddCell(c44);
    dataGrid->AddRow(row4);
    
    auto row5 = std::make_shared<DataGridRow>();
    auto c51 = std::make_shared<DataGridCell>(); c51->SetText(L"Laptop Stand"); row5->AddCell(c51);
    auto c52 = std::make_shared<DataGridCell>(); c52->SetText(L"Furniture"); row5->AddCell(c52);
    auto c53 = std::make_shared<DataGridCell>(); c53->SetText(L"$45.00"); row5->AddCell(c53);
    auto c54 = std::make_shared<DataGridCell>(); c54->SetText(L"89"); row5->AddCell(c54);
    dataGrid->AddRow(row5);
    
    if (auto* l = dataGrid->GetLayout()) {
        l->SetHeight(160);
    }
    gridCard->AddChild(dataGrid);
    root->AddChild(gridCard);
    
    // TabControl 示例区域
    auto tabCard = CreateCard(L"Nested TabControl", L"Tab controls can be nested");
    
    auto tabControl = std::make_shared<TabControl>();
    tabControl->SetTabHeight(28);
    
    auto tab1 = std::make_shared<TabItem>();
    tab1->SetHeader(L"General");
    auto t1c = std::make_shared<TextBlock>();
    t1c->SetText(L"General settings and preferences");
    tab1->SetContent(t1c);
    tabControl->AddTab(tab1);
    
    auto tab2 = std::make_shared<TabItem>();
    tab2->SetHeader(L"Advanced");
    auto t2c = std::make_shared<TextBlock>();
    t2c->SetText(L"Advanced configuration options");
    tab2->SetContent(t2c);
    tabControl->AddTab(tab2);
    
    auto tab3 = std::make_shared<TabItem>();
    tab3->SetHeader(L"About");
    auto t3c = std::make_shared<TextBlock>();
    t3c->SetText(L"Application version and credits");
    tab3->SetContent(t3c);
    tabControl->AddTab(tab3);
    
    if (auto* l = tabControl->GetLayout()) {
        l->SetHeight(100);
    }
    tabCard->AddChild(tabControl);
    root->AddChild(tabCard);
    
    return root;
}

// ========== Graphics 页 ==========
std::shared_ptr<Panel> ControlsShowcaseWindow::CreateGraphicsTab() {
    auto root = std::make_shared<StackPanel>();
    root->SetOrientation(StackPanel::Orientation::Vertical);
    root->SetSpacing(20);
    
    // Shapes 区域
    auto shapeCard = CreateCard(L"Shapes", L"Basic vector shapes with fill and stroke");
    
    auto shapesPanel = std::make_shared<StackPanel>();
    shapesPanel->SetOrientation(StackPanel::Orientation::Horizontal);
    shapesPanel->SetSpacing(32);
    
    // Rectangle
    auto rectCont = std::make_shared<StackPanel>();
    rectCont->SetOrientation(StackPanel::Orientation::Vertical);
    rectCont->SetSpacing(6);
    auto rect = std::make_shared<luaui::controls::Rectangle>();
    if (auto* l = rect->GetLayout()) { l->SetWidth(64); l->SetHeight(64); }
    rect->SetFill(Color::FromHex(0x4CAF50));
    rect->SetStroke(Color::FromHex(0x2E7D32));
    rect->SetStrokeThickness(2);
    rectCont->AddChild(rect);
    auto rl = std::make_shared<TextBlock>();
    rl->SetText(L"Rectangle");
    rl->SetFontSize(10);
    rl->SetForeground(Color::FromHex(0x757575));
    rectCont->AddChild(rl);
    shapesPanel->AddChild(rectCont);
    
    // Ellipse
    auto ellCont = std::make_shared<StackPanel>();
    ellCont->SetOrientation(StackPanel::Orientation::Vertical);
    ellCont->SetSpacing(6);
    auto ellipse = std::make_shared<luaui::controls::Ellipse>();
    if (auto* l = ellipse->GetLayout()) { l->SetWidth(64); l->SetHeight(64); }
    ellipse->SetFill(Color::FromHex(0x2196F3));
    ellipse->SetStroke(Color::FromHex(0x1565C0));
    ellipse->SetStrokeThickness(2);
    ellCont->AddChild(ellipse);
    auto el = std::make_shared<TextBlock>();
    el->SetText(L"Ellipse");
    el->SetFontSize(10);
    el->SetForeground(Color::FromHex(0x757575));
    ellCont->AddChild(el);
    shapesPanel->AddChild(ellCont);
    
    // Circle with thicker border
    auto circCont = std::make_shared<StackPanel>();
    circCont->SetOrientation(StackPanel::Orientation::Vertical);
    circCont->SetSpacing(6);
    auto circle = std::make_shared<luaui::controls::Ellipse>();
    if (auto* l = circle->GetLayout()) { l->SetWidth(64); l->SetHeight(64); }
    circle->SetFill(Color::FromHex(0xFF5722));
    circle->SetStroke(Color::FromHex(0xE64A19));
    circle->SetStrokeThickness(4);
    circCont->AddChild(circle);
    auto cl = std::make_shared<TextBlock>();
    cl->SetText(L"Circle");
    cl->SetFontSize(10);
    cl->SetForeground(Color::FromHex(0x757575));
    circCont->AddChild(cl);
    shapesPanel->AddChild(circCont);
    
    // Rounded rect simulation (using Border)
    auto borderCont = std::make_shared<StackPanel>();
    borderCont->SetOrientation(StackPanel::Orientation::Vertical);
    borderCont->SetSpacing(6);
    auto border = std::make_shared<Border>();
    if (auto* l = border->GetLayout()) { l->SetWidth(64); l->SetHeight(64); }
    border->SetBackground(Color::FromHex(0x9C27B0));
    border->SetBorderColor(Color::FromHex(0x7B1FA2));
    border->SetBorderThickness(2);
    borderCont->AddChild(border);
    auto bl = std::make_shared<TextBlock>();
    bl->SetText(L"Border");
    bl->SetFontSize(10);
    bl->SetForeground(Color::FromHex(0x757575));
    borderCont->AddChild(bl);
    shapesPanel->AddChild(borderCont);
    
    shapeCard->AddChild(shapesPanel);
    root->AddChild(shapeCard);
    
    // Colors 区域
    auto colorCard = CreateCard(L"Color Palette", L"Material Design color samples");
    
    auto colorRow1 = CreateRow();
    auto colorNames = {L"Red", L"Pink", L"Purple", L"Deep Purple", L"Indigo", L"Blue"};
    auto colorVals = {0xF44336, 0xE91E63, 0x9C27B0, 0x673AB7, 0x3F51B5, 0x2196F3};
    
    auto namesIt = colorNames.begin();
    auto valsIt = colorVals.begin();
    for (; namesIt != colorNames.end() && valsIt != colorVals.end(); ++namesIt, ++valsIt) {
        auto c = std::make_shared<luaui::controls::Rectangle>();
        if (auto* l = c->GetLayout()) { l->SetWidth(40); l->SetHeight(40); }
        c->SetFill(Color::FromHex(*valsIt));
        colorRow1->AddChild(c);
    }
    colorCard->AddChild(colorRow1);
    
    auto colorRow2 = CreateRow();
    auto colorNames2 = {L"Cyan", L"Teal", L"Green", L"Lime", L"Yellow", L"Orange"};
    auto colorVals2 = {0x00BCD4, 0x009688, 0x4CAF50, 0xCDDC39, 0xFFEB3B, 0xFF9800};
    
    namesIt = colorNames2.begin();
    valsIt = colorVals2.begin();
    for (; namesIt != colorNames2.end() && valsIt != colorVals2.end(); ++namesIt, ++valsIt) {
        auto c = std::make_shared<luaui::controls::Rectangle>();
        if (auto* l = c->GetLayout()) { l->SetWidth(40); l->SetHeight(40); }
        c->SetFill(Color::FromHex(*valsIt));
        colorRow2->AddChild(c);
    }
    colorCard->AddChild(colorRow2);
    
    root->AddChild(colorCard);
    
    return root;
}

// ========== 窗口加载 ==========
void ControlsShowcaseWindow::OnLoaded() {
    Logger::Info("Controls Showcase Starting");
    
    // 根面板
    auto root = std::make_shared<StackPanel>();
    root->SetOrientation(StackPanel::Orientation::Vertical);
    if (auto* l = root->GetLayout()) {
        l->SetPadding(20, 16, 20, 16);
    }
    if (auto* r = root->GetRender()) {
        r->SetBackground(Color::FromHex(0xFAFAFA));
    }
    
    // 标题区
    auto title = std::make_shared<TextBlock>();
    title->SetText(L"LuaUI Controls Showcase");
    title->SetFontSize(28);
    title->SetForeground(Color::FromHex(0x212121));
    title->SetFontWeight(FontWeight::Bold);
    root->AddChild(title);
    
    auto subtitle = std::make_shared<TextBlock>();
    subtitle->SetText(L"Comprehensive UI component demonstration with modern design");
    subtitle->SetFontSize(13);
    subtitle->SetForeground(Color::FromHex(0x757575));
    root->AddChild(subtitle);
    
    // 主 TabControl
    auto mainTabs = std::make_shared<TabControl>();
    mainTabs->SetTabHeight(36);
    
    auto tabBasic = std::make_shared<TabItem>();
    tabBasic->SetHeader(L"  Basic  ");
    tabBasic->SetContent(CreateBasicTab());
    mainTabs->AddTab(tabBasic);
    
    auto tabInput = std::make_shared<TabItem>();
    tabInput->SetHeader(L"  Input  ");
    tabInput->SetContent(CreateInputTab());
    mainTabs->AddTab(tabInput);
    
    auto tabSelect = std::make_shared<TabItem>();
    tabSelect->SetHeader(L" Selection ");
    tabSelect->SetContent(CreateSelectionTab());
    mainTabs->AddTab(tabSelect);
    
    auto tabData = std::make_shared<TabItem>();
    tabData->SetHeader(L"  Data   ");
    tabData->SetContent(CreateDataTab());
    mainTabs->AddTab(tabData);
    
    auto tabGraphics = std::make_shared<TabItem>();
    tabGraphics->SetHeader(L" Graphics ");
    tabGraphics->SetContent(CreateGraphicsTab());
    mainTabs->AddTab(tabGraphics);
    
    if (auto* l = mainTabs->GetLayout()) {
        l->SetHeight(460);
    }
    root->AddChild(mainTabs);
    
    // 状态栏
    auto statusPanel = std::make_shared<StackPanel>();
    statusPanel->SetOrientation(StackPanel::Orientation::Vertical);
    statusPanel->SetSpacing(4);
    if (auto* l = statusPanel->GetLayout()) {
        l->SetPadding(12, 8, 12, 8);
        l->SetMargin(0, 12, 0, 0);
    }
    if (auto* r = statusPanel->GetRender()) {
        r->SetBackground(Color::White());
    }
    
    auto st = std::make_shared<TextBlock>();
    st->SetText(L"Status");
    st->SetFontSize(10);
    st->SetForeground(Color::FromHex(0x9E9E9E));
    st->SetFontWeight(FontWeight::SemiBold);
    statusPanel->AddChild(st);
    
    m_statusText = std::make_shared<TextBlock>();
    m_statusText->SetText(L"Ready - Interact with controls to see feedback");
    m_statusText->SetFontSize(12);
    m_statusText->SetForeground(Color::FromHex(0x424242));
    statusPanel->AddChild(m_statusText);
    
    root->AddChild(statusPanel);
    
    SetRoot(root);
    Logger::Info("Showcase initialized");
}

// ============================================================================
// Entry Point
// ============================================================================

int main() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    
    Logger::Initialize();
    Logger::SetConsoleLevel(LogLevel::Debug);
    
    try {
        ControlsShowcaseWindow window;
        
        if (!window.Create(hInstance, L"LuaUI Controls Showcase", 800, 720)) {
            Logger::Error("Failed to create window");
            return 1;
        }
        
        window.Show(SW_SHOW);
        return window.Run();
        
    } catch (const std::exception& e) {
        Logger::ErrorF("Exception: %s", e.what());
        return 1;
    }
}
