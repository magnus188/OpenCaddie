#include "domain/Settings.h"

#include <array>
#include <cmath>

namespace opencaddie::domain {

std::optional<std::string> validateSettings(const Settings& settings) {
    if (settings.language != "en" && settings.language != "nb") {
        return "language must be en or nb";
    }
    if (settings.palette != "dark" && settings.palette != "light") {
        return "palette must be dark or light";
    }
    if (!std::isfinite(settings.textScale) || settings.textScale < 0.8 ||
        settings.textScale > 1.5) {
        return "text scale must be between 0.8 and 1.5";
    }
    if (settings.brightnessPercent < 10 || settings.brightnessPercent > 100) {
        return "brightness must be between 10 and 100";
    }
    if (settings.cacheLimitMegabytes < 128 ||
        settings.cacheLimitMegabytes > 12'000) {
        return "cache limit must be between 128 and 12000 MB";
    }
    if (!std::isfinite(settings.recommendationBiasMetres) ||
        settings.recommendationBiasMetres < -30.0 ||
        settings.recommendationBiasMetres > 30.0) {
        return "recommendation bias must be between -30 and 30 metres";
    }
    const bool validServer =
        settings.openGolfMapServer.starts_with("https://") ||
        settings.openGolfMapServer.starts_with("http://localhost") ||
        settings.openGolfMapServer.starts_with("http://127.0.0.1");
    if (!validServer) {
        return "OpenGolfMap server must use HTTPS (localhost may use HTTP)";
    }
    return std::nullopt;
}

} // namespace opencaddie::domain

