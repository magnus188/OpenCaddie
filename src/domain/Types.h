#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace opencaddie::domain {

enum class UnitSystem { Metric, Imperial };
enum class ScoringMode { StrokePlay, Stableford };
enum class RoundStatus { InProgress, Completed, Abandoned };
enum class FairwayResult { NotRecorded, Left, Centre, Right, Missed };

struct GeoPoint {
    double latitude = 0.0;
    double longitude = 0.0;
};

struct PositionFix {
    GeoPoint point;
    double accuracyMetres = 0.0;
    std::chrono::system_clock::time_point timestamp{};
    bool valid = false;
};

struct HoleDefinition {
    int number = 1;
    int par = 4;
    int strokeIndex = 1;
};

struct HoleScore {
    int hole = 1;
    int strokes = 0;
    std::optional<int> putts;
    int penalties = 0;
    FairwayResult fairway = FairwayResult::NotRecorded;
    std::optional<bool> greenInRegulation;
    std::string tee;
    std::string notes;
};

struct Club {
    std::string id;
    std::string name;
    double carryMetres = 0.0;
    bool enabled = true;
    int position = 0;
};

struct ClubAdvice {
    Club club;
    double targetMetres = 0.0;
    double deltaMetres = 0.0;
};

struct GreenDistances {
    double frontMetres = 0.0;
    double centreMetres = 0.0;
    double backMetres = 0.0;
};

struct LocalProjection {
    GeoPoint origin;
    double earthRadiusMetres = 6'378'137.0;
    double rotationRadians = 0.0;
};

} // namespace opencaddie::domain

