#pragma once

#include <optional>
#include <string>

namespace opencaddie::domain {

struct WeatherConditions {
    std::optional<double> temperatureC;
    std::optional<double> windSpeedMps;
    std::optional<int> windFromDegrees;  // meteorological "from" direction
    std::string condition;               // storage weather_condition code
};

struct PlaysLikeBreakdown {
    double baseMetres = 0.0;
    double windMetres = 0.0;         // + = plays longer
    double temperatureMetres = 0.0;  // + = plays longer
    double conditionMetres = 0.0;    // + = plays longer
    [[nodiscard]] double totalMetres() const;
    [[nodiscard]] bool hasAdjustments() const;
};

PlaysLikeBreakdown computePlaysLike(double baseDistanceMetres,
                                    double shotBearingDegrees,
                                    const WeatherConditions& weather);

} // namespace opencaddie::domain
