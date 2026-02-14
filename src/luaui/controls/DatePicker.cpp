#include "DatePicker.h"
#include "Components/LayoutComponent.h"
#include "Components/RenderComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/IRenderable.h"
#include "Interfaces/ILayoutable.h"
#include "IRenderContext.h"
#include <ctime>
#include <sstream>
#include <iomanip>

namespace luaui {
namespace controls {

// ============================================================================
// DatePicker
// ============================================================================
DatePicker::DatePicker() {
    // 默认选中今天
    m_selectedDate = std::chrono::system_clock::now();
    
    // 默认范围：1900-2100
    std::tm minTm = {};
    minTm.tm_year = 0;    // 1900
    minTm.tm_mon = 0;
    minTm.tm_mday = 1;
    m_minDate = std::chrono::system_clock::from_time_t(std::mktime(&minTm));
    
    std::tm maxTm = {};
    maxTm.tm_year = 200;  // 2100
    maxTm.tm_mon = 11;
    maxTm.tm_mday = 31;
    m_maxDate = std::chrono::system_clock::from_time_t(std::mktime(&maxTm));
}

void DatePicker::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
    
    // 创建日历控件
    m_calendar = std::make_shared<Calendar>();
    m_calendar->SetSelectedDate(m_selectedDate);
    m_calendar->SetDateRange(m_minDate, m_maxDate);
    
    // 监听日历选择事件
    m_calendar->SelectedDateChanged.Add([this](Calendar*) {
        SetSelectedDate(m_calendar->GetSelectedDate());
        CloseDropDown();
    });
}

void DatePicker::SetSelectedDate(const std::chrono::system_clock::time_point& date) {
    // 限制在范围内
    auto clampedDate = date;
    if (clampedDate < m_minDate) clampedDate = m_minDate;
    if (clampedDate > m_maxDate) clampedDate = m_maxDate;
    
    if (m_selectedDate != clampedDate) {
        m_selectedDate = clampedDate;
        if (m_calendar) {
            m_calendar->SetSelectedDate(clampedDate);
        }
        SelectedDateChanged.Invoke(this);
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

int DatePicker::GetYear() const {
    std::time_t tt = std::chrono::system_clock::to_time_t(m_selectedDate);
    std::tm localTm = {};
    localtime_s(&localTm, &tt);
    return localTm.tm_year + 1900;
}

int DatePicker::GetMonth() const {
    std::time_t tt = std::chrono::system_clock::to_time_t(m_selectedDate);
    std::tm localTm = {};
    localtime_s(&localTm, &tt);
    return localTm.tm_mon + 1;
}

int DatePicker::GetDay() const {
    std::time_t tt = std::chrono::system_clock::to_time_t(m_selectedDate);
    std::tm localTm = {};
    localtime_s(&localTm, &tt);
    return localTm.tm_mday;
}

void DatePicker::SetDate(int year, int month, int day) {
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    
    std::time_t tt = std::mktime(&tm);
    if (tt != -1) {
        SetSelectedDate(std::chrono::system_clock::from_time_t(tt));
    }
}

void DatePicker::SetShowsTime(bool shows) {
    if (m_showsTime != shows) {
        m_showsTime = shows;
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

int DatePicker::GetHour() const {
    std::time_t tt = std::chrono::system_clock::to_time_t(m_selectedDate);
    std::tm localTm = {};
    localtime_s(&localTm, &tt);
    return localTm.tm_hour;
}

int DatePicker::GetMinute() const {
    std::time_t tt = std::chrono::system_clock::to_time_t(m_selectedDate);
    std::tm localTm = {};
    localtime_s(&localTm, &tt);
    return localTm.tm_min;
}

void DatePicker::SetTime(int hour, int minute) {
    std::time_t tt = std::chrono::system_clock::to_time_t(m_selectedDate);
    std::tm localTm = {};
    localtime_s(&localTm, &tt);
    localTm.tm_hour = hour;
    localTm.tm_min = minute;
    tt = std::mktime(&localTm);
    if (tt != -1) {
        SetSelectedDate(std::chrono::system_clock::from_time_t(tt));
    }
}

void DatePicker::SetIsDropDownOpen(bool open) {
    if (m_isDropDownOpen != open) {
        m_isDropDownOpen = open;
        if (open) {
            OpenDropDown();
        } else {
            CloseDropDown();
        }
        DropDownOpenedChanged.Invoke(this, open);
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void DatePicker::ToggleDropDown() {
    SetIsDropDownOpen(!m_isDropDownOpen);
}

void DatePicker::OpenDropDown() {
    if (m_calendar) {
        // 更新日历显示为选中日期
        m_calendar->SetSelectedDate(m_selectedDate);
        
        // 计算下拉位置
        rendering::Rect rect;
        if (auto* renderable = AsRenderable()) {
            rect = renderable->GetRenderRect();
        }
        
        // 定位日历在下拉框下方
        if (auto* calLayoutable = m_calendar->AsLayoutable()) {
            calLayoutable->Arrange(rendering::Rect(rect.x, rect.y + rect.height, 
                                                    m_dropDownWidth, m_dropDownHeight));
        }
        m_calendar->SetIsVisible(true);
    }
}

void DatePicker::CloseDropDown() {
    if (m_calendar) {
        m_calendar->SetIsVisible(false);
    }
}

void DatePicker::UpdateVisualState() {
    if (auto* render = GetRender()) {
        render->Invalidate();
    }
}

std::wstring DatePicker::FormatDate(const std::chrono::system_clock::time_point& date) const {
    std::time_t tt = std::chrono::system_clock::to_time_t(date);
    std::tm localTm = {};
    localtime_s(&localTm, &tt);
    
    switch (m_dateFormat) {
        case DateFormat::ShortDate: {
            std::wstringstream ss;
            ss << (localTm.tm_year + 1900) << L"/" 
               << (localTm.tm_mon + 1) << L"/" 
               << localTm.tm_mday;
            return ss.str();
        }
        case DateFormat::LongDate: {
            std::wstringstream ss;
            ss << (localTm.tm_year + 1900) << L"-" 
               << (localTm.tm_mon + 1) << L"-" 
               << localTm.tm_mday;
            return ss.str();
        }
        case DateFormat::Custom: {
            // 简化：仅支持基本的 yyyy, MM, dd 替换
            std::wstring result = m_customFormat;
            
            // 替换 yyyy
            size_t pos = result.find(L"yyyy");
            if (pos != std::wstring::npos) {
                result.replace(pos, 4, std::to_wstring(localTm.tm_year + 1900));
            }
            
            // 替换 MM
            pos = result.find(L"MM");
            if (pos != std::wstring::npos) {
                std::wstring month = std::to_wstring(localTm.tm_mon + 1);
                if (month.length() < 2) month = L"0" + month;
                result.replace(pos, 2, month);
            }
            
            // 替换 dd
            pos = result.find(L"dd");
            if (pos != std::wstring::npos) {
                std::wstring day = std::to_wstring(localTm.tm_mday);
                if (day.length() < 2) day = L"0" + day;
                result.replace(pos, 2, day);
            }
            
            return result;
        }
    }
    return L"";
}

void DatePicker::OnClick() {
    ToggleDropDown();
}

void DatePicker::OnMouseEnter() {
    m_isHovered = true;
    UpdateVisualState();
}

void DatePicker::OnMouseLeave() {
    m_isHovered = false;
    UpdateVisualState();
}

void DatePicker::DrawDropDownArrow(rendering::IRenderContext* context, const rendering::Rect& rect) {
    auto brush = context->CreateSolidColorBrush(m_arrowColor);
    if (!brush) return;
    
    float centerX = rect.x + rect.width / 2;
    float centerY = rect.y + rect.height / 2;
    float size = 4.0f;
    
    if (m_isDropDownOpen) {
        // 向上箭头
        context->DrawLine(rendering::Point(centerX - size, centerY + size/2),
                          rendering::Point(centerX, centerY - size/2),
                          brush.get(), 1.0f);
        context->DrawLine(rendering::Point(centerX, centerY - size/2),
                          rendering::Point(centerX + size, centerY + size/2),
                          brush.get(), 1.0f);
    } else {
        // 向下箭头
        context->DrawLine(rendering::Point(centerX - size, centerY - size/2),
                          rendering::Point(centerX, centerY + size/2),
                          brush.get(), 1.0f);
        context->DrawLine(rendering::Point(centerX, centerY + size/2),
                          rendering::Point(centerX + size, centerY - size/2),
                          brush.get(), 1.0f);
    }
}

rendering::Size DatePicker::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    
    // 估算最小宽度
    float minWidth = 120.0f + m_arrowWidth + m_padding * 2;
    return rendering::Size(minWidth, 32.0f);
}

void DatePicker::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景
    rendering::Color borderColor = m_isHovered ? m_borderHoverColor : m_borderColor;
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    auto borderBrush = context->CreateSolidColorBrush(borderColor);
    
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    if (borderBrush) {
        context->DrawRectangle(rect, borderBrush.get(), 1.0f);
    }
    
    // 绘制日期文本或水印
    std::wstring displayText;
    rendering::Color textColor;
    
    // 检查是否有有效日期（这里简化：如果日期是默认的最小值，显示水印）
    std::time_t tt = std::chrono::system_clock::to_time_t(m_selectedDate);
    std::tm localTm = {};
    localtime_s(&localTm, &tt);
    
    if (localTm.tm_year <= 70) { // 1970年之前认为是无效日期
        displayText = m_watermark;
        textColor = m_watermarkColor;
    } else {
        displayText = FormatDate(m_selectedDate);
        textColor = m_textColor;
    }
    
    if (!displayText.empty()) {
        auto textBrush = context->CreateSolidColorBrush(textColor);
        auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
        
        if (textBrush && textFormat) {
            rendering::Point textPos(rect.x + m_padding, 
                                     rect.y + (rect.height - m_fontSize) / 2);
            context->DrawTextString(displayText, textFormat.get(), textPos, textBrush.get());
        }
    }
    
    // 绘制下拉箭头
    rendering::Rect arrowRect(rect.x + rect.width - m_arrowWidth, rect.y,
                               m_arrowWidth, rect.height);
    DrawDropDownArrow(context, arrowRect);
    
    // 绘制日历下拉（如果打开）
    if (m_isDropDownOpen && m_calendar) {
        if (auto* calRenderable = m_calendar->AsRenderable()) {
            calRenderable->Render(context);
        }
    }
}

// ============================================================================
// Calendar
// ============================================================================
Calendar::Calendar() {
    // 默认显示当前月
    std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm localTm = {};
    localtime_s(&localTm, &tt);
    m_displayYear = localTm.tm_year + 1900;
    m_displayMonth = localTm.tm_mon + 1;
}

void Calendar::InitializeComponents() {
    GetComponents().AddComponent<components::LayoutComponent>(this);
    GetComponents().AddComponent<components::RenderComponent>(this);
    GetComponents().AddComponent<components::InputComponent>(this);
    
    SetIsVisible(false);
}

void Calendar::SetDisplayDate(int year, int month) {
    if (m_displayYear != year || m_displayMonth != month) {
        m_displayYear = year;
        m_displayMonth = month;
        
        // 规范化月份
        while (m_displayMonth > 12) {
            m_displayMonth -= 12;
            m_displayYear++;
        }
        while (m_displayMonth < 1) {
            m_displayMonth += 12;
            m_displayYear--;
        }
        
        DisplayDateChanged.Invoke(this, m_displayYear, m_displayMonth);
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Calendar::SetSelectedDate(const std::chrono::system_clock::time_point& date) {
    if (m_selectedDate != date) {
        m_selectedDate = date;
        
        // 更新显示月份为选中日期所在月
        std::time_t tt = std::chrono::system_clock::to_time_t(date);
        std::tm localTm = {};
        localtime_s(&localTm, &tt);
        SetDisplayDate(localTm.tm_year + 1900, localTm.tm_mon + 1);
        
        SelectedDateChanged.Invoke(this);
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void Calendar::SetDateRange(const std::chrono::system_clock::time_point& minDate,
                            const std::chrono::system_clock::time_point& maxDate) {
    m_minDate = minDate;
    m_maxDate = maxDate;
}

void Calendar::PreviousMonth() {
    SetDisplayDate(m_displayYear, m_displayMonth - 1);
}

void Calendar::NextMonth() {
    SetDisplayDate(m_displayYear, m_displayMonth + 1);
}

void Calendar::PreviousYear() {
    SetDisplayDate(m_displayYear - 1, m_displayMonth);
}

void Calendar::NextYear() {
    SetDisplayDate(m_displayYear + 1, m_displayMonth);
}

int Calendar::GetDaysInMonth(int year, int month) const {
    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2) {
        // 闰年判断
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return isLeap ? 29 : 28;
    }
    return daysInMonth[month - 1];
}

int Calendar::GetFirstDayOfWeek(int year, int month) const {
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = 1;
    std::mktime(&tm);
    return tm.tm_wday; // 0=周日, 6=周六
}

bool Calendar::IsDateInRange(int year, int month, int day) const {
    auto date = MakeDate(year, month, day);
    return date >= m_minDate && date <= m_maxDate;
}

std::chrono::system_clock::time_point Calendar::MakeDate(int year, int month, int day) const {
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    std::time_t tt = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(tt);
}

int Calendar::HitTestDay(float x, float y) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    float daysY = rect.y + m_headerHeight + m_dayOfWeekHeight;
    float dayWidth = rect.width / 7;
    
    if (y < daysY || y >= daysY + 6 * m_dayHeight) return -1;
    if (x < rect.x || x >= rect.x + rect.width) return -1;
    
    int col = static_cast<int>((x - rect.x) / dayWidth);
    int row = static_cast<int>((y - daysY) / m_dayHeight);
    
    if (col < 0 || col >= 7 || row < 0 || row >= 6) return -1;
    
    int firstDay = GetFirstDayOfWeek(m_displayYear, m_displayMonth);
    int daysInMonth = GetDaysInMonth(m_displayYear, m_displayMonth);
    
    int day = row * 7 + col - firstDay + 1;
    if (day < 1 || day > daysInMonth) return -1;
    
    return day;
}

int Calendar::HitTestHeaderButton(float x, float y) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    if (y < rect.y || y > rect.y + m_headerHeight) return -1;
    
    float buttonWidth = 24.0f;
    float yearButtonX = rect.x + buttonWidth;
    float monthNextX = rect.x + rect.width - buttonWidth * 2;
    float yearNextX = rect.x + rect.width - buttonWidth;
    
    if (x >= rect.x && x < rect.x + buttonWidth) return 0; // 上月
    if (x >= yearButtonX && x < yearButtonX + buttonWidth) return 2; // 上年
    if (x >= monthNextX && x < monthNextX + buttonWidth) return 1; // 下月
    if (x >= yearNextX && x < yearNextX + buttonWidth) return 3; // 下年
    
    return -1;
}

void Calendar::RenderHeader(rendering::IRenderContext* context, const rendering::Rect& rect) {
    // 绘制头部背景
    auto bgBrush = context->CreateSolidColorBrush(m_headerColor);
    if (bgBrush) {
        context->FillRectangle(rendering::Rect(rect.x, rect.y, rect.width, m_headerHeight),
                               bgBrush.get());
    }
    
    // 绘制年月标题
    auto textBrush = context->CreateSolidColorBrush(m_dayColor);
    auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
    
    if (textBrush && textFormat) {
        textFormat->SetTextAlignment(rendering::TextAlignment::Center);
        
        std::wstringstream ss;
        ss << m_displayYear << L"-" << m_displayMonth;
        
        rendering::Point textPos(rect.x + rect.width / 2, 
                                 rect.y + (m_headerHeight - m_fontSize) / 2);
        context->DrawTextString(ss.str(), textFormat.get(), textPos, textBrush.get());
    }
    
    // 绘制导航按钮
    auto buttonBrush = context->CreateSolidColorBrush(
        m_hoveredHeaderButton >= 0 ? m_buttonHoverColor : m_buttonColor);
    if (buttonBrush) {
        float buttonY = rect.y + (m_headerHeight - 10) / 2;
        float buttonWidth = 24.0f;
        
        // 左箭头（上月）
        float leftX = rect.x + buttonWidth / 2;
        context->DrawLine(rendering::Point(leftX - 3, buttonY),
                          rendering::Point(leftX + 2, buttonY + 5),
                          buttonBrush.get(), 1.5f);
        context->DrawLine(rendering::Point(leftX + 2, buttonY + 5),
                          rendering::Point(leftX - 3, buttonY + 10),
                          buttonBrush.get(), 1.5f);
        
        // 右箭头（下月）
        float rightX = rect.x + rect.width - buttonWidth / 2 - buttonWidth;
        context->DrawLine(rendering::Point(rightX + 3, buttonY),
                          rendering::Point(rightX - 2, buttonY + 5),
                          buttonBrush.get(), 1.5f);
        context->DrawLine(rendering::Point(rightX - 2, buttonY + 5),
                          rendering::Point(rightX + 3, buttonY + 10),
                          buttonBrush.get(), 1.5f);
        
        // 双箭头（上年/下年）简化表示
    }
}

void Calendar::RenderDaysOfWeek(rendering::IRenderContext* context, const rendering::Rect& rect) {
    const wchar_t* days[] = {L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat"};
    
    auto textBrush = context->CreateSolidColorBrush(m_dayOfWeekColor);
    auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_smallFontSize);
    
    if (textBrush && textFormat) {
        textFormat->SetTextAlignment(rendering::TextAlignment::Center);
        
        float dayWidth = rect.width / 7;
        float y = rect.y + m_headerHeight + (m_dayOfWeekHeight - m_smallFontSize) / 2;
        
        for (int i = 0; i < 7; ++i) {
            rendering::Point textPos(rect.x + i * dayWidth + dayWidth / 2, y);
            context->DrawTextString(days[i], textFormat.get(), textPos, textBrush.get());
        }
    }
}

void Calendar::RenderDays(rendering::IRenderContext* context, const rendering::Rect& rect) {
    int firstDay = GetFirstDayOfWeek(m_displayYear, m_displayMonth);
    int daysInMonth = GetDaysInMonth(m_displayYear, m_displayMonth);
    
    float dayWidth = rect.width / 7;
    float daysY = rect.y + m_headerHeight + m_dayOfWeekHeight;
    
    auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", m_fontSize);
    if (!textFormat) return;
    textFormat->SetTextAlignment(rendering::TextAlignment::Center);
    textFormat->SetParagraphAlignment(rendering::ParagraphAlignment::Center);
    
    // 获取今天的日期
    std::time_t now = std::time(nullptr);
    std::tm todayTm = {};
    localtime_s(&todayTm, &now);
    int todayYear = todayTm.tm_year + 1900;
    int todayMonth = todayTm.tm_mon + 1;
    int todayDay = todayTm.tm_mday;
    
    // 获取选中的日期
    int selYear = 0, selMonth = 0, selDay = 0;
    if (m_selectedDate != std::chrono::system_clock::time_point()) {
        std::time_t sel = std::chrono::system_clock::to_time_t(m_selectedDate);
        std::tm selTm = {};
        localtime_s(&selTm, &sel);
        selYear = selTm.tm_year + 1900;
        selMonth = selTm.tm_mon + 1;
        selDay = selTm.tm_mday;
    }
    
    for (int day = 1; day <= daysInMonth; ++day) {
        int pos = firstDay + day - 1;
        int row = pos / 7;
        int col = pos % 7;
        
        float x = rect.x + col * dayWidth;
        float y = daysY + row * m_dayHeight;
        rendering::Rect dayRect(x, y, dayWidth, m_dayHeight);
        
        bool isToday = (m_displayYear == todayYear && m_displayMonth == todayMonth && day == todayDay);
        bool isSelected = (m_displayYear == selYear && m_displayMonth == selMonth && day == selDay);
        bool isHovered = (day == m_hoveredDay);
        bool isInRange = IsDateInRange(m_displayYear, m_displayMonth, day);
        
        // 绘制背景
        if (isSelected) {
            auto bgBrush = context->CreateSolidColorBrush(m_selectedDayBg);
            if (bgBrush) {
                context->FillRectangle(dayRect, bgBrush.get());
            }
        } else if (isHovered && isInRange) {
            auto bgBrush = context->CreateSolidColorBrush(m_hoverDayBg);
            if (bgBrush) {
                context->FillRectangle(dayRect, bgBrush.get());
            }
        }
        
        // 绘制今天边框
        if (isToday) {
            auto borderBrush = context->CreateSolidColorBrush(m_todayBorderColor);
            if (borderBrush) {
                context->DrawRectangle(dayRect, borderBrush.get(), 1.5f);
            }
        }
        
        // 绘制日期文本
        rendering::Color textColor;
        if (!isInRange) {
            textColor = m_disabledDayColor;
        } else if (isSelected) {
            textColor = m_selectedDayColor;
        } else {
            textColor = m_dayColor;
        }
        
        auto textBrush = context->CreateSolidColorBrush(textColor);
        if (textBrush) {
            std::wstring dayStr = std::to_wstring(day);
            rendering::Point textPos(x + dayWidth / 2, y + m_dayHeight / 2 - m_fontSize / 2);
            context->DrawTextString(dayStr, textFormat.get(), textPos, textBrush.get());
        }
    }
}

rendering::Size Calendar::OnMeasure(const rendering::Size& availableSize) {
    (void)availableSize;
    
    // 固定大小
    float width = 280.0f;
    float height = m_headerHeight + m_dayOfWeekHeight + 6 * m_dayHeight;
    
    return rendering::Size(width, height);
}

void Calendar::OnRender(rendering::IRenderContext* context) {
    if (!context) return;
    
    auto* render = GetRender();
    if (!render) return;
    
    auto rect = render->GetRenderRect();
    
    // 绘制背景
    auto bgBrush = context->CreateSolidColorBrush(m_bgColor);
    auto borderBrush = context->CreateSolidColorBrush(m_borderColor);
    
    if (bgBrush) {
        context->FillRectangle(rect, bgBrush.get());
    }
    if (borderBrush) {
        context->DrawRectangle(rect, borderBrush.get(), 1.0f);
    }
    
    // 绘制头部
    RenderHeader(context, rect);
    
    // 绘制星期标题
    RenderDaysOfWeek(context, rect);
    
    // 绘制日期
    RenderDays(context, rect);
}

void Calendar::OnMouseMove(MouseEventArgs& args) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    int prevHoveredDay = m_hoveredDay;
    int prevHoveredButton = m_hoveredHeaderButton;
    
    m_hoveredDay = HitTestDay(args.x, args.y);
    m_hoveredHeaderButton = HitTestHeaderButton(args.x, args.y);
    
    if (prevHoveredDay != m_hoveredDay || prevHoveredButton != m_hoveredHeaderButton) {
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
    
    args.Handled = true;
}

void Calendar::OnMouseDown(MouseEventArgs& args) {
    rendering::Rect rect;
    if (auto* renderable = AsRenderable()) {
        rect = renderable->GetRenderRect();
    }
    
    // 检查是否点击日期
    int day = HitTestDay(args.x, args.y);
    if (day > 0 && IsDateInRange(m_displayYear, m_displayMonth, day)) {
        SetSelectedDate(MakeDate(m_displayYear, m_displayMonth, day));
    }
    
    // 检查是否点击导航按钮
    int button = HitTestHeaderButton(args.x, args.y);
    switch (button) {
        case 0: PreviousMonth(); break;
        case 1: NextMonth(); break;
        case 2: PreviousYear(); break;
        case 3: NextYear(); break;
    }
    
    args.Handled = true;
}

} // namespace controls
} // namespace luaui
