#include <fmt/format.h>

#include "FormatUtils.h"

std::string FormatUtils::formatShutterSpeed(float shutter)
{
    if (shutter < 1.0f)
        return fmt::format("1/{}s", std::round(1.0f / shutter));
    return fmt::format("{}s", shutter);
}

std::string FormatUtils::formatAperture(float aperture)
{
    auto rounded = std::round(aperture * 10) / 10;
    return fmt::format("f/{}", rounded);
}

std::string FormatUtils::formatISO(float iso)
{
    return fmt::format("ISO {}", std::round(iso));
}

std::string FormatUtils::formatFocalLength(float focal)
{
    return fmt::format("{}mm", std::round(focal));
}

std::string FormatUtils::formatResolution(unsigned int resolution)
{
    return fmt::format("{}MP", std::round(static_cast<float>(resolution) / 1000000.0f));
}

std::string FormatUtils::formatLens(const libraw_lensinfo_t& lens)
{
    if (stringEmpty(lens.LensMake) && stringEmpty(lens.Lens))
        return "Unknown lens";
    if (stringEmpty(lens.LensMake))
        return lens.Lens;
    return fmt::format("{} {}", lens.LensMake, lens.Lens);
}
