#pragma once

#include "macros.h"
#include <vector>

struct Data {
    std::vector<std::time_t> timestamps {};
    std::vector<float> iso_speeds {};
    std::vector<float> shutter_speeds {};
    std::vector<float> focal_lengths {};
    std::vector<float> aperture_values {};
    std::vector<float> resolutions {};
    std::map<std::string, std::size_t> lenses {};
    std::map<std::string, std::size_t> cameras {};

    /// @brief Reserves the capacity for all vectors (not for lens and camera maps)
    void reserveCapacity(std::size_t capacity)
    {
        timestamps.reserve(capacity);
        iso_speeds.reserve(capacity);
        shutter_speeds.reserve(capacity);
        focal_lengths.reserve(capacity);
        aperture_values.reserve(capacity);
        resolutions.reserve(capacity);
    }

    void assertEqualSizes() const
    {
        ASSERT(iso_speeds.size() == timestamps.size());
        ASSERT(iso_speeds.size() == shutter_speeds.size());
        ASSERT(iso_speeds.size() == focal_lengths.size());
        ASSERT(iso_speeds.size() == aperture_values.size());
        ASSERT(iso_speeds.size() == resolutions.size());
    }

    void assertNotEmpty() const
    {
        ASSERT(!timestamps.empty());
        ASSERT(!iso_speeds.empty());
        ASSERT(!shutter_speeds.empty());
        ASSERT(!focal_lengths.empty());
        ASSERT(!aperture_values.empty());
        ASSERT(!resolutions.empty());
        ASSERT(!lenses.empty());
        ASSERT(!cameras.empty());
    }
};
