#include "domain/Geo.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace opencaddie::domain {
namespace {
constexpr double Pi = 3.14159265358979323846;
constexpr double DegreesToRadians = Pi / 180.0;

double dot(const double ax, const double ay, const double bx, const double by) {
    return ax * bx + ay * by;
}
} // namespace

double haversineMetres(const GeoPoint& from, const GeoPoint& to) {
    constexpr double EarthRadiusMetres = 6'371'008.8;
    const double lat1 = from.latitude * DegreesToRadians;
    const double lat2 = to.latitude * DegreesToRadians;
    const double deltaLat = (to.latitude - from.latitude) * DegreesToRadians;
    const double deltaLon = (to.longitude - from.longitude) * DegreesToRadians;
    const double a = std::pow(std::sin(deltaLat / 2.0), 2.0) +
                     std::cos(lat1) * std::cos(lat2) *
                         std::pow(std::sin(deltaLon / 2.0), 2.0);
    return 2.0 * EarthRadiusMetres *
           std::atan2(std::sqrt(a), std::sqrt(std::max(0.0, 1.0 - a)));
}

double initialBearingDegrees(const GeoPoint& from, const GeoPoint& to) {
    const double lat1 = from.latitude * DegreesToRadians;
    const double lat2 = to.latitude * DegreesToRadians;
    const double deltaLon = (to.longitude - from.longitude) * DegreesToRadians;
    const double y = std::sin(deltaLon) * std::cos(lat2);
    const double x = std::cos(lat1) * std::sin(lat2) -
                     std::sin(lat1) * std::cos(lat2) * std::cos(deltaLon);
    const double bearing = std::atan2(y, x) / DegreesToRadians;
    return std::fmod(bearing + 360.0, 360.0);
}

std::pair<double, double> projectToLocal(const GeoPoint& point,
                                        const LocalProjection& projection) {
    const double cosLatitude =
        std::cos(projection.origin.latitude * DegreesToRadians);
    const double projectedX =
        (point.longitude - projection.origin.longitude) * DegreesToRadians *
        projection.earthRadiusMetres * cosLatitude;
    const double projectedY =
        -(point.latitude - projection.origin.latitude) * DegreesToRadians *
        projection.earthRadiusMetres;
    const double cosine = std::cos(projection.rotationRadians);
    const double sine = std::sin(projection.rotationRadians);
    return {projectedX * cosine - projectedY * sine,
            projectedX * sine + projectedY * cosine};
}

GeoPoint unprojectFromLocal(const double x, const double y,
                            const LocalProjection& projection) {
    const double cosine = std::cos(projection.rotationRadians);
    const double sine = std::sin(projection.rotationRadians);
    const double unrotatedX = x * cosine + y * sine;
    const double unrotatedY = -x * sine + y * cosine;
    const double cosLatitude =
        std::cos(projection.origin.latitude * DegreesToRadians);
    return {
        projection.origin.latitude -
            unrotatedY /
                (DegreesToRadians * projection.earthRadiusMetres),
        projection.origin.longitude +
            unrotatedX /
                (DegreesToRadians * projection.earthRadiusMetres * cosLatitude),
    };
}

GreenDistances distancesToGreen(const GeoPoint& player,
                                const std::span<const GeoPoint> greenPolygon,
                                const GeoPoint& greenCentre) {
    const double centreDistance = haversineMetres(player, greenCentre);
    if (greenPolygon.empty()) {
        return {centreDistance, centreDistance, centreDistance};
    }

    LocalProjection projection{player};
    const auto [centreX, centreY] = projectToLocal(greenCentre, projection);
    const double centreMagnitude = std::hypot(centreX, centreY);
    if (centreMagnitude < 0.01) {
        double furthest = 0.0;
        for (const auto& point : greenPolygon) {
            furthest = std::max(furthest, haversineMetres(player, point));
        }
        return {0.0, 0.0, furthest};
    }

    const double axisX = centreX / centreMagnitude;
    const double axisY = centreY / centreMagnitude;
    double nearestAlongAxis = std::numeric_limits<double>::infinity();
    double furthestAlongAxis = -std::numeric_limits<double>::infinity();
    for (const auto& point : greenPolygon) {
        const auto [x, y] = projectToLocal(point, projection);
        const double alongAxis = dot(x, y, axisX, axisY);
        nearestAlongAxis = std::min(nearestAlongAxis, alongAxis);
        furthestAlongAxis = std::max(furthestAlongAxis, alongAxis);
    }
    return {std::max(0.0, nearestAlongAxis), centreDistance,
            std::max(centreDistance, furthestAlongAxis)};
}

bool isUsableFix(const PositionFix& fix,
                 const std::chrono::system_clock::time_point now,
                 const double maximumAccuracyMetres,
                 const std::chrono::seconds maximumAge) {
    return fix.valid && std::isfinite(fix.accuracyMetres) &&
           fix.accuracyMetres >= 0.0 &&
           fix.accuracyMetres <= maximumAccuracyMetres &&
           fix.timestamp <= now && now - fix.timestamp <= maximumAge &&
           fix.point.latitude >= -90.0 && fix.point.latitude <= 90.0 &&
           fix.point.longitude >= -180.0 && fix.point.longitude <= 180.0;
}

} // namespace opencaddie::domain

