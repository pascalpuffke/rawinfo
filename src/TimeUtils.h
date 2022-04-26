#pragma once

#include <ctime>
#include <string>

class TimeUtils final {
public:
    static std::string formatISO8601(std::time_t);
    static std::string formatTimeSpan(std::time_t start, std::time_t end);
    static std::string formatTimeSince(std::time_t);
};
