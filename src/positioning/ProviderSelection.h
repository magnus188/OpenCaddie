#pragma once

namespace opencaddie::positioning {

enum class ProviderMode { NoFix, RouteReplay, Gpsd };

[[nodiscard]] constexpr ProviderMode selectProviderMode(
    const bool simulator, const bool demoRound, const bool explicitRoute) {
    if (!simulator) return ProviderMode::Gpsd;
    return demoRound || explicitRoute ? ProviderMode::RouteReplay
                                      : ProviderMode::NoFix;
}

} // namespace opencaddie::positioning
