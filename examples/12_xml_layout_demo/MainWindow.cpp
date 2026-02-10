#include "MainWindow.h"
#include "Logger.h"
#include <fstream>
#include <sstream>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;
using namespace luaui::xml;
using luaui::utils::Logger;

namespace demo {

MainWindow::MainWindow() { InitializeBindings(); }
MainWindow::~MainWindow() = default;

void MainWindow::InitializeBindings() {
    RegisterControl("newBtn", &m_newBtn); RegisterControl("openBtn", &m_openBtn);
    RegisterControl("saveBtn", &m_saveBtn); RegisterControl("searchBtn", &m_searchBtn);
    RegisterControl("submitBtn", &m_submitBtn); RegisterControl("cancelBtn", &m_cancelBtn);
    RegisterControl("resetBtn", &m_resetBtn); RegisterControl("settingsBtn", &m_settingsBtn);
    RegisterControl("navHomeBtn", &m_navHomeBtn); RegisterControl("navProfileBtn", &m_navProfileBtn);
    RegisterControl("usernameBox", &m_usernameBox); RegisterControl("emailBox", &m_emailBox);
    RegisterControl("bioBox", &m_bioBox); RegisterControl("volumeSlider", &m_volumeSlider);
    RegisterControl("profileProgress", &m_profileProgress);
    RegisterControl("statusText", &m_statusText); RegisterControl("progressPercentText", &m_progressPercentText);
}

bool MainWindow::Initialize(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW); wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc; wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"LuaUIXmlMainWindow";
    if (!RegisterClassExW(&wcex)) return false;

    m_hWnd = CreateWindowExW(0, L"LuaUIXmlMainWindow", L"XML Layout Demo",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 950, 780, nullptr, nullptr, hInstance, this);
    if (!m_hWnd) return false;

    m_engine = CreateRenderEngine();
    if (!m_engine || !m_engine->Initialize()) return false;

    RenderTargetDesc desc; desc.type = RenderTargetType::Window;
    desc.nativeHandle = m_hWnd; desc.width = 950; desc.height = 780;
    if (!m_engine->CreateRenderTarget(desc)) return false;

    CreateFromXml();
    ShowWindow(m_hWnd, nCmdShow); UpdateWindow(m_hWnd);
    return true;
}

void MainWindow::CreateFromXml() {
    try {
        Logger::Info("Loading XML...");
        m_xmlLoader = CreateXmlLoader();
        m_root = m_xmlLoader->Load("layouts/main_window.xml");
        FindAndBindControls(); WireUpEvents();
        UpdateStatus(L"Ready"); UpdateProgress(65.0f);
    } catch (const std::exception& e) {
        Logger::ErrorF("XML Error: %s", e.what());
    }
}

void MainWindow::FindAndBindControls() {
    if (!m_root) return;
    size_t found = 0;
    std::function<void(ControlPtr)> find = [&](ControlPtr c) {
        if (!c) return;
        std::string name = c->GetName();
        if (!name.empty()) {
            auto it = m_controlBindings.find(name);
            if (it != m_controlBindings.end()) { it->second.binder(c.get()); found++; }
        }
        for (size_t i = 0; i < c->GetChildCount(); i++) find(c->GetChild(i));
    };
    find(m_root);
    Logger::InfoF("Found %zu controls", found);
}

void MainWindow::WireUpEvents() {
    struct Event { Button* btn; std::function<void()> h; const char* n; };
    Event events[] = {{m_newBtn, [this](){OnNewClick();}, "new"}, {m_openBtn, [this](){OnOpenClick();}, "open"},
        {m_saveBtn, [this](){OnSaveClick();}, "save"}, {m_searchBtn, [this](){OnSearchClick();}, "search"},
        {m_submitBtn, [this](){OnSubmitClick();}, "submit"}, {m_cancelBtn, [this](){OnCancelClick();}, "cancel"},
        {m_resetBtn, [this](){OnResetClick();}, "reset"}, {m_settingsBtn, [this](){OnSettingsClick();}, "settings"},
        {m_navHomeBtn, [this](){OnNavHomeClick();}, "home"}, {m_navProfileBtn, [this](){OnNavProfileClick();}, "profile"}};
    for (auto& e : events) if (e.btn) e.btn->AddClickHandler([h=e.h](Control*){h();});
    if (m_volumeSlider) m_volumeSlider->SetValueChangedHandler([this](Slider*,double v){OnSliderValueChanged((float)v);});
}

void MainWindow::OnNewClick() { UpdateStatus(L"New"); UpdateProgress(0); }
void MainWindow::OnOpenClick() { UpdateStatus(L"Open"); UpdateProgress(85); }
void MainWindow::OnSaveClick() { UpdateStatus(L"Save"); }
void MainWindow::OnSearchClick() { UpdateStatus(L"Search"); }
void MainWindow::OnSubmitClick() { UpdateStatus(L"Submit"); UpdateProgress(100); }
void MainWindow::OnCancelClick() { UpdateStatus(L"Cancel"); }
void MainWindow::OnResetClick() { UpdateStatus(L"Reset"); UpdateProgress(0); }
void MainWindow::OnSettingsClick() { UpdateStatus(L"Settings"); }
void MainWindow::OnNavHomeClick() { UpdateStatus(L"Home"); }
void MainWindow::OnNavProfileClick() { UpdateStatus(L"Profile"); }
void MainWindow::OnSliderValueChanged(float v) { UpdateStatus(L"Volume: " + std::to_wstring((int)v) + L"%"); }

void MainWindow::UpdateStatus(const std::wstring& msg) {
    if (m_statusText) m_statusText->SetText(msg);
    Logger::InfoF("%ls", msg.c_str());
}
void MainWindow::UpdateProgress(float p) {
    if (m_profileProgress) m_profileProgress->SetValue(p);
    if (m_progressPercentText) m_progressPercentText->SetText(std::to_wstring((int)p) + L"%");
}

void MainWindow::Render() {
    if (!m_engine->BeginFrame()) return;
    auto* ctx = m_engine->GetContext(); if (!ctx) { m_engine->Present(); return; }
    ctx->Clear(Color::FromHex(0xF5F5F5));
    RECT rc; GetClientRect(m_hWnd, &rc);
    if (m_root) { m_root->Measure(Size((float)rc.right, (float)rc.bottom));
        m_root->Arrange(Rect(0, 0, (float)rc.right, (float)rc.bottom)); m_root->Render(ctx); }
    m_engine->Present();
}
int MainWindow::Run() { MSG msg; while (GetMessage(&msg, nullptr, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); } return (int)msg.wParam; }

LRESULT CALLBACK MainWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM lP) {
    MainWindow* p = (msg == WM_NCCREATE) ? reinterpret_cast<MainWindow*>(((CREATESTRUCT*)lP)->lpCreateParams) : 
        reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE) { SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)p); p->m_hWnd = hWnd; }
    if (!p) return DefWindowProc(hWnd, msg, wP, lP);
    switch (msg) {
        case WM_PAINT: { PAINTSTRUCT ps; BeginPaint(hWnd, &ps); p->Render(); EndPaint(hWnd, &ps); return 0; }
        case WM_SIZE: if (p->m_engine) p->m_engine->ResizeRenderTarget(LOWORD(lP), HIWORD(lP)); InvalidateRect(hWnd, nullptr, FALSE); return 0;
        case WM_LBUTTONDOWN: if (p->m_root) { auto hit = p->m_root->HitTestPoint(Point((float)GET_X_LPARAM(lP), (float)GET_Y_LPARAM(lP))); if (hit) { hit->OnMouseDown(MouseEventArgs((float)GET_X_LPARAM(lP), (float)GET_Y_LPARAM(lP))); if (auto* b = dynamic_cast<Button*>(hit.get())) b->RaiseClick(); } } InvalidateRect(hWnd, nullptr, FALSE); return 0;
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hWnd, msg, wP, lP);
}
} // namespace demo
