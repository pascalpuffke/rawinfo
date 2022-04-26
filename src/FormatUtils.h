#pragma once

#include <libraw/libraw_types.h>
#include <string>

class FormatUtils final {
public:
    [[nodiscard]] static std::string formatShutterSpeed(float);
    [[nodiscard]] static std::string formatAperture(float);
    [[nodiscard]] static std::string formatISO(float);
    [[nodiscard]] static std::string formatFocalLength(float);
    [[nodiscard]] static std::string formatResolution(unsigned int);
    [[nodiscard]] static std::string formatLens(const libraw_lensinfo_t&);

private:
    static constexpr bool stringEmpty(const char* str)
    {
        return str == nullptr || str[0] == '\0';
    }
};
