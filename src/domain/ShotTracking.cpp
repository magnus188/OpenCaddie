#include "domain/ShotTracking.h"

#include "domain/Geo.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace opencaddie::domain {
namespace {

double distanceToSegment(const double x, const double y, const double ax,
                         const double ay, const double bx, const double by) {
    const double dx = bx - ax;
    const double dy = by - ay;
    const double lengthSquared = dx * dx + dy * dy;
    if (lengthSquared <= std::numeric_limits<double>::epsilon())
        return std::hypot(x - ax, y - ay);
    const double projection =
        std::clamp(((x - ax) * dx + (y - ay) * dy) / lengthSquared, 0.0, 1.0);
    return std::hypot(x - (ax + projection * dx),
                      y - (ay + projection * dy));
}

struct PolygonRelation {
    bool valid = false;
    bool inside = false;
    double distanceMetres = std::numeric_limits<double>::infinity();
};

bool validPoint(const GeoPoint &point) {
    return std::isfinite(point.latitude) && std::isfinite(point.longitude) &&
           point.latitude >= -90.0 && point.latitude <= 90.0 &&
           point.longitude >= -180.0 && point.longitude <= 180.0;
}

PolygonRelation relationToPolygon(const GeoPoint &point,
                                  const std::span<const GeoPoint> polygon) {
    if (polygon.size() < 3)
        return {};

    const LocalProjection projection{point};
    std::vector<std::pair<double, double>> local;
    local.reserve(polygon.size());
    for (const auto &vertex : polygon) {
        if (!validPoint(vertex))
            return {};
        local.push_back(projectToLocal(vertex, projection));
        if (!std::isfinite(local.back().first) ||
            !std::isfinite(local.back().second)) {
            return {};
        }
    }

    bool inside = false;
    double distance = std::numeric_limits<double>::infinity();
    double twiceArea = 0.0;
    for (std::size_t index = 0, previous = local.size() - 1;
         index < local.size(); previous = index++) {
        const auto [x1, y1] = local[previous];
        const auto [x2, y2] = local[index];
        twiceArea += x1 * y2 - x2 * y1;
        distance = std::min(distance, distanceToSegment(0.0, 0.0, x1, y1, x2, y2));
        const bool crosses = (y1 > 0.0) != (y2 > 0.0);
        if (crosses) {
            const double intersectionX =
                x1 + (x2 - x1) * (-y1) / (y2 - y1);
            if (intersectionX >= 0.0)
                inside = !inside;
        }
    }
    if (!std::isfinite(twiceArea) || std::abs(twiceArea) < 0.01 ||
        !std::isfinite(distance)) {
        return {};
    }
    // Treat the polygon boundary as green. The tolerance also absorbs tiny
    // projection/serialization errors in course packages.
    if (distance <= 0.25)
        inside = true;
    return {true, inside, distance};
}

} // namespace

std::string_view strokeTypeKey(const StrokeType type) {
    switch (type) {
    case StrokeType::Drive:
        return "drive";
    case StrokeType::Approach:
        return "approach";
    case StrokeType::Chip:
        return "chip";
    case StrokeType::Putt:
        return "putt";
    case StrokeType::Unknown:
        return "unknown";
    }
    return "unknown";
}

std::optional<StrokeType> strokeTypeFromKey(const std::string_view key) {
    if (key == "drive")
        return StrokeType::Drive;
    if (key == "approach")
        return StrokeType::Approach;
    if (key == "chip")
        return StrokeType::Chip;
    if (key == "putt")
        return StrokeType::Putt;
    if (key == "unknown")
        return StrokeType::Unknown;
    return std::nullopt;
}

StrokeType classifyTrackedStroke(const int par, const int scoreBeforeStroke,
                                 const int recordedStrokeCount,
                                 const bool landingPositionAvailable,
                                 const std::optional<GeoPoint> &start,
                                 const std::span<const GeoPoint> greenPolygon,
                                 const double chipThresholdMetres) {
    if (!landingPositionAvailable || scoreBeforeStroke < 0 ||
        recordedStrokeCount < 0 || !std::isfinite(chipThresholdMetres) ||
        chipThresholdMetres < 0.0) {
        return StrokeType::Unknown;
    }
    if (!start || !validPoint(*start) || greenPolygon.size() < 3)
        return StrokeType::Unknown;
    const PolygonRelation relation = relationToPolygon(*start, greenPolygon);
    if (!relation.valid)
        return StrokeType::Unknown;
    if (scoreBeforeStroke == 0 && recordedStrokeCount == 0) {
        if (par == 3)
            return StrokeType::Approach;
        if (par >= 4)
            return StrokeType::Drive;
        return StrokeType::Unknown;
    }
    if (relation.inside)
        return StrokeType::Putt;
    if (relation.distanceMetres <= chipThresholdMetres)
        return StrokeType::Chip;
    return StrokeType::Approach;
}

} // namespace opencaddie::domain
