#pragma once

#include "domain/Types.h"

#include <optional>
#include <string>

namespace opencaddie::domain {

struct Settings {
    UnitSystem units = UnitSystem::Metric;
    std::string language = "en";
    std::string palette = "dark";
    double textScale = 1.0;
    int brightnessPercent = 80;
    int cacheLimitMegabytes = 1024;
    double recommendationBiasMetres = 0.0;
    bool automaticHoleAdvance = true;
    std::string openGolfMapServer = "https://opengolfmap.example";
};

std::optional<std::string> validateSettings(const Settings& settings);

} // namespace opencaddie::domain
