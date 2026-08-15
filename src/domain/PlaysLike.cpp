#include "domain/PlaysLike.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace opencaddie::domain {
namespace {
constexpr double Pi = 3.14159265358979323846;
constexpr double DegreesToRadians = Pi / 180.0;

// Effect model constants. All effects scale with the base distance so a wedge
// shot is adjusted less than a driver in absolute metres.
// A 1 m/s headwind component lengthens the shot by ~1%; the same tailwind
// shortens it by only half as much (lift/drag asymmetry).
constexpr double HeadwindFactorPerMps = 0.010;
constexpr double TailwindFactorPerMps = 0.005;
// Colder, denser air carries less: ~1% per 8°C below the 20°C reference.
constexpr double ReferenceTemperatureC = 20.0;
constexpr double TemperatureFactorPerDegree = 0.0013;
// Wet conditions cost carry and roll-out.
constexpr double RainFactor = 0.015;

bool isWetCondition(const std::string& condition) {
    static constexpr std::array wet{"rain", "drizzle", "showers", "thunderstorm"};
    return std::find(wet.begin(), wet.end(), condition) != wet.end();
}
} // namespace

double PlaysLikeBreakdown::totalMetres() const {
    return std::max(0.0, baseMetres + windMetres + temperatureMetres + conditionMetres);
}

bool PlaysLikeBreakdown::hasAdjustments() const {
    return windMetres != 0.0 || temperatureMetres != 0.0 || conditionMetres != 0.0;
}

PlaysLikeBreakdown computePlaysLike(const double baseDistanceMetres,
                                    const double shotBearingDegrees,
                                    const WeatherConditions& weather) {
    PlaysLikeBreakdown breakdown;
    breakdown.baseMetres = std::max(0.0, baseDistanceMetres);
    if (breakdown.baseMetres <= 0.0)
        return breakdown;

    if (weather.windSpeedMps && weather.windFromDegrees) {
        // Positive component = wind blowing from the target toward the player,
        // i.e. a headwind.
        const double relative =
            (static_cast<double>(*weather.windFromDegrees) - shotBearingDegrees) *
            DegreesToRadians;
        const double headwindMps = *weather.windSpeedMps * std::cos(relative);
        const double factor =
            headwindMps >= 0.0 ? HeadwindFactorPerMps : TailwindFactorPerMps;
        breakdown.windMetres = breakdown.baseMetres * headwindMps * factor;
    }

    if (weather.temperatureC) {
        breakdown.temperatureMetres = breakdown.baseMetres *
                                      (ReferenceTemperatureC - *weather.temperatureC) *
                                      TemperatureFactorPerDegree;
    }

    if (isWetCondition(weather.condition))
        breakdown.conditionMetres = breakdown.baseMetres * RainFactor;

    return breakdown;
}

} // namespace opencaddie::domain
