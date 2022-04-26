#include <fmt/chrono.h>

#include "TimeUtils.h"
#include "macros.h"

using Clock = std::chrono::system_clock;
using namespace std::chrono_literals;

// Workaround for incomplete std chrono implementation :^)
constexpr auto day = 24h;
constexpr auto week = 7 * day;
constexpr auto month = 30 * day; // Let's just assume that a month is always 30 days
constexpr auto year = 365 * day; // Let's not bother with leap years

constexpr auto secondsInMinute = 60;
constexpr auto secondsInHour = 60 * secondsInMinute;
constexpr auto secondsInDay = 24 * secondsInHour;
constexpr auto secondsInWeek = 7 * secondsInDay;
constexpr auto secondsInMonth = 30 * secondsInDay;
constexpr auto secondsInYear = 365 * secondsInDay;

template <typename TimeUnit = std::chrono::seconds>
constexpr auto timeDifference(const std::time_t& start, const std::time_t& end)
{
    return std::chrono::duration_cast<TimeUnit>(Clock::from_time_t(end) - Clock::from_time_t(start));
}

ALWAYS_INLINE std::string formatEnding(const char* unit, long number, const char* postfix = "ago")
{
    if (number == 1)
        return fmt::format("{} {} {}", number, unit, postfix);
    return fmt::format("{} {}s {}", number, unit, postfix);
}

std::string TimeUtils::formatISO8601(const std::time_t timestamp)
{
    return fmt::format("{:%Y-%m-%dT%H:%M:%S%z}", fmt::localtime(timestamp));
}

std::string TimeUtils::formatTimeSpan(const std::time_t start, const std::time_t end)
{
    const auto duration = timeDifference(start, end);
    const auto seconds = duration.count();
    if (duration < 1s)
        return "less than a second";
    if (duration < 1min)
        return fmt::format("{} seconds", seconds);
    if (duration < 1h) {
        if (seconds % secondsInMinute == 0)
            return fmt::format("{} minutes", duration / 1min);
        return fmt::format("{} minutes, {} seconds", duration / 1min, duration % 1min);
    }
    // Now the chrono library begins to fall apart, yay!
    if (duration < day) {
        if (seconds % secondsInHour == 0)
            return fmt::format("{} hours", duration / 1h);
        return fmt::format("{} hours, {} minutes", duration / 1h, duration % 1h / 1min);
    }
    if (duration < month) {
        if (seconds % secondsInDay == 0)
            return fmt::format("{} days", duration / day);
        return fmt::format("{} days, {} hours", duration / day, duration % day / 1h);
    }
    // To make it even worse, after this point we get inaccuracies!
    if (duration < year) {
        if (seconds % secondsInMonth == 0)
            return fmt::format("{} months", duration / month);
        return fmt::format("approx. {} months, {} days", duration / month, duration % month / day);
    }
    if (seconds % secondsInYear == 0)
        return fmt::format("{} years", duration / year);
    return fmt::format("approx. {} years, {} months", duration / year, duration % year / month);
}

std::string TimeUtils::formatTimeSince(const std::time_t timestamp)
{
    auto duration = timeDifference(timestamp, Clock::to_time_t(Clock::now()));
    auto seconds = duration.count();

    if (duration < 1s)
        return "just now";
    if (duration < 1min)
        return formatEnding("second", seconds);
    if (duration < 1h)
        return formatEnding("minute", seconds / secondsInMinute);
    if (duration < day)
        return formatEnding("hour", seconds / secondsInHour);
    if (duration < week)
        return formatEnding("day", seconds / secondsInDay);
    if (duration < month)
        return formatEnding("week", seconds / secondsInWeek);
    if (duration < year)
        return formatEnding("month", seconds / secondsInMonth);
    return formatEnding("year", seconds / secondsInYear);
}
