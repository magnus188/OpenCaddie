#pragma once

#include "domain/Types.h"

#include <optional>
#include <span>
#include <string_view>

namespace opencaddie::domain {

enum class StrokeType { Drive, Approach, Chip, Putt, Unknown };

std::string_view strokeTypeKey(StrokeType type);
std::optional<StrokeType> strokeTypeFromKey(std::string_view key);

StrokeType classifyTrackedStroke(int par, int scoreBeforeStroke,
                                 int recordedStrokeCount,
                                 bool landingPositionAvailable,
                                 const std::optional<GeoPoint> &start,
                                 std::span<const GeoPoint> greenPolygon,
                                 double chipThresholdMetres = 50.0);

} // namespace opencaddie::domain
