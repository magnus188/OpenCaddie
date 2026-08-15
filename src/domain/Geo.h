#pragma once

#include "domain/Types.h"

#include <span>
#include <utility>

namespace opencaddie::domain {

double haversineMetres(const GeoPoint& from, const GeoPoint& to);
// Initial great-circle bearing, degrees clockwise from true north, [0, 360).
double initialBearingDegrees(const GeoPoint& from, const GeoPoint& to);
std::pair<double, double> projectToLocal(const GeoPoint& point,
                                        const LocalProjection& projection);
GeoPoint unprojectFromLocal(double x, double y,
                            const LocalProjection& projection);
GreenDistances distancesToGreen(const GeoPoint& player,
                                std::span<const GeoPoint> greenPolygon,
                                const GeoPoint& greenCentre);
bool isUsableFix(const PositionFix& fix,
                 std::chrono::system_clock::time_point now,
                 double maximumAccuracyMetres = 25.0,
                 std::chrono::seconds maximumAge = std::chrono::seconds{10});

} // namespace opencaddie::domain

