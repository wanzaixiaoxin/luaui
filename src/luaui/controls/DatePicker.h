#pragma once

#include "Control.h"
#include "../rendering/Types.h"
#include <chrono>
#include <string>
#include <functional>

namespace luaui {
namespace controls {

// 前向声明
class Calendar;

/**
 * @brief DatePicker 日期选择器（新架构）
 * 
 * 支持：
 * - 日期选择（年/月/日）
 * - 日期时间选择（可选）
 * - 日期范围限制
 * - 自定义日期格式
 * - 日历下拉
 */
class DatePicker : public luaui::Control {
public:
    // 日期显示格式
    enum class DateFormat {
        ShortDate,      // 短日期（如 2024/1/15）
        LongDate,       // 长日期（如 2024年1月15日）
        Custom          // 自定义格式
    };
    
    DatePicker();
    
    std::string GetTypeName() const override { return "DatePicker"; }
    
    // 选中日期
    std::chrono::system_clock::time_point GetSelectedDate() const { return m_selectedDate; }
    void SetSelectedDate(const std::chrono::system_clock::time_point& date);
    
    // 获取日期各部分
    int GetYear() const;
    int GetMonth() const;   // 1-12
    int GetDay() const;     // 1-31
    
    // 设置日期（便捷方法）
    void SetDate(int year, int month, int day);
    
    // 是否显示时间
    bool GetShowsTime() const { return m_showsTime; }
    void SetShowsTime(bool shows);
    
    // 时间部分
    int GetHour() const;    // 0-23
    int GetMinute() const;  // 0-59
    void SetTime(int hour, int minute);
    
    // 日期范围
    std::chrono::system_clock::time_point GetMinimumDate() const { return m_minDate; }
    void SetMinimumDate(const std::chrono::system_clock::time_point& date) { m_minDate = date; }
    
    std::chrono::system_clock::time_point GetMaximumDate() const { return m_maxDate; }
    void SetMaximumDate(const std::chrono::system_clock::time_point& date) { m_maxDate = date; }
    
    // 显示格式
    DateFormat GetDateFormat() const { return m_dateFormat; }
    void SetDateFormat(DateFormat format) { m_dateFormat = format; }
    
    std::wstring GetCustomFormat() const { return m_customFormat; }
    void SetCustomFormat(const std::wstring& format) { m_customFormat = format; }
    
    // 日历下拉
    bool GetIsDropDownOpen() const { return m_isDropDownOpen; }
    void SetIsDropDownOpen(bool open);
    
    // 占位符文本
    std::wstring GetWatermark() const { return m_watermark; }
    void SetWatermark(const std::wstring& text) { m_watermark = text; }
    
    // 事件
    luaui::Delegate<DatePicker*> SelectedDateChanged;
    luaui::Delegate<DatePicker*, bool> DropDownOpenedChanged;
    
    // 便捷方法：格式化日期
    std::wstring FormatDate(const std::chrono::system_clock::time_point& date) const;

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    
    void OnClick() override;
    void OnMouseEnter() override;
    void OnMouseLeave() override;

private:
    void ToggleDropDown();
    void OpenDropDown();
    void CloseDropDown();
    void UpdateVisualState();
    void DrawDropDownArrow(rendering::IRenderContext* context, const rendering::Rect& rect);
    
    std::chrono::system_clock::time_point m_selectedDate;
    std::chrono::system_clock::time_point m_minDate;
    std::chrono::system_clock::time_point m_maxDate;
    
    bool m_showsTime = false;
    bool m_isDropDownOpen = false;
    DateFormat m_dateFormat = DateFormat::ShortDate;
    std::wstring m_customFormat = L"yyyy-MM-dd";
    std::wstring m_watermark = L"选择日期...";
    
    // 状态
    bool m_isHovered = false;
    bool m_isPressed = false;
    
    // 外观
    float m_padding = 8.0f;
    float m_arrowWidth = 20.0f;
    float m_fontSize = 14.0f;
    float m_dropDownWidth = 280.0f;
    float m_dropDownHeight = 280.0f;
    
    // 颜色
    rendering::Color m_borderColor = rendering::Color::FromHex(0x8E8E8E);
    rendering::Color m_borderHoverColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_bgColor = rendering::Color::White();
    rendering::Color m_textColor = rendering::Color::Black();
    rendering::Color m_watermarkColor = rendering::Color::FromHex(0x999999);
    rendering::Color m_arrowColor = rendering::Color::FromHex(0x666666);
    
    // 日历控件（下拉时显示）
    std::shared_ptr<Calendar> m_calendar;
};

/**
 * @brief Calendar 日历控件
 * 
 * 用于 DatePicker 的下拉日历，也可独立使用
 */
class Calendar : public luaui::Control {
public:
    Calendar();
    
    std::string GetTypeName() const override { return "Calendar"; }
    
    // 显示月份
    int GetDisplayYear() const { return m_displayYear; }
    int GetDisplayMonth() const { return m_displayMonth; }
    void SetDisplayDate(int year, int month);
    
    // 选中日期
    std::chrono::system_clock::time_point GetSelectedDate() const { return m_selectedDate; }
    void SetSelectedDate(const std::chrono::system_clock::time_point& date);
    
    // 日期范围
    void SetDateRange(const std::chrono::system_clock::time_point& minDate,
                      const std::chrono::system_clock::time_point& maxDate);
    
    // 事件
    luaui::Delegate<Calendar*> SelectedDateChanged;
    luaui::Delegate<Calendar*, int, int> DisplayDateChanged; // year, month

protected:
    void InitializeComponents() override;
    void OnRender(rendering::IRenderContext* context) override;
    rendering::Size OnMeasure(const rendering::Size& availableSize) override;
    
    void OnMouseMove(MouseEventArgs& args) override;
    void OnMouseDown(MouseEventArgs& args) override;

private:
    void RenderHeader(rendering::IRenderContext* context, const rendering::Rect& rect);
    void RenderDaysOfWeek(rendering::IRenderContext* context, const rendering::Rect& rect);
    void RenderDays(rendering::IRenderContext* context, const rendering::Rect& rect);
    
    void PreviousMonth();
    void NextMonth();
    void PreviousYear();
    void NextYear();
    
    int HitTestDay(float x, float y);
    int HitTestHeaderButton(float x, float y);
    
    int GetDaysInMonth(int year, int month) const;
    int GetFirstDayOfWeek(int year, int month) const; // 0=周日, 6=周六
    bool IsDateInRange(int year, int month, int day) const;
    
    std::chrono::system_clock::time_point MakeDate(int year, int month, int day) const;
    
    int m_displayYear = 2024;
    int m_displayMonth = 1;
    std::chrono::system_clock::time_point m_selectedDate;
    std::chrono::system_clock::time_point m_minDate;
    std::chrono::system_clock::time_point m_maxDate;
    
    int m_hoveredDay = -1;
    int m_hoveredHeaderButton = -1; // 0=上月, 1=下月, 2=上年, 3=下年
    
    // 外观
    float m_headerHeight = 36.0f;
    float m_dayOfWeekHeight = 24.0f;
    float m_dayHeight = 32.0f;
    float m_padding = 4.0f;
    float m_fontSize = 14.0f;
    float m_smallFontSize = 12.0f;
    
    // 颜色
    rendering::Color m_bgColor = rendering::Color::White();
    rendering::Color m_headerColor = rendering::Color::FromHex(0xF5F5F5);
    rendering::Color m_dayOfWeekColor = rendering::Color::FromHex(0x666666);
    rendering::Color m_dayColor = rendering::Color::Black();
    rendering::Color m_otherMonthDayColor = rendering::Color::FromHex(0xAAAAAA);
    rendering::Color m_selectedDayColor = rendering::Color::White();
    rendering::Color m_selectedDayBg = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_hoverDayBg = rendering::Color::FromHex(0xE5F3FF);
    rendering::Color m_todayBorderColor = rendering::Color::FromHex(0x0078D4);
    rendering::Color m_disabledDayColor = rendering::Color::FromHex(0xCCCCCC);
    rendering::Color m_borderColor = rendering::Color::FromHex(0xCCCCCC);
    rendering::Color m_buttonColor = rendering::Color::FromHex(0x666666);
    rendering::Color m_buttonHoverColor = rendering::Color::FromHex(0x0078D4);
};

} // namespace controls
} // namespace luaui
