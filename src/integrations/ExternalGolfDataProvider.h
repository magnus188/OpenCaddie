#pragma once

#include <chrono>
#include <cstddef>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace opencaddie::integrations {

enum class ProviderOperation {
    ReadRounds,
    ReadHoleScores,
    ReadShots,
    ReadLaunchMetrics,
    ReadWeather,
    ReadHandicap,
    ImportCsv,
    ImportFit,
    ExportPrivateScorecard,
    SubmitWhsScore,
    CorrectWhsScore,
    SearchTeeTimes,
    CreateBooking,
    CancelBooking,
    ProcessBookingPayment,
};

using ProviderOperations = std::set<ProviderOperation>;

enum class ValueOrigin { Unknown, Measured, Derived, UserEntered };
enum class ExternalFairwayResult {
    NotRecorded,
    Left,
    Centre,
    Right,
    Missed,
};

struct Provenance {
    std::string provider;
    std::string accountId;
    std::string externalId;
    std::string device;
    std::string sourceSchema;
    std::optional<std::chrono::system_clock::time_point> sourceTime;
    std::string attribution;
    std::string rawChecksumSha256;
    ValueOrigin origin = ValueOrigin::Unknown;
};

struct Wgs84Position {
    double latitude = 0.0;
    double longitude = 0.0;
    std::optional<double> accuracyMetres;
};

struct MetricValue {
    double canonicalValue = 0.0;
    std::string canonicalUnit;
    std::optional<double> sourceValue;
    std::string sourceUnit;
    ValueOrigin origin = ValueOrigin::Unknown;
};

struct ExternalHoleScore {
    int hole = 1;
    int par = 0;
    int strokes = 0;
    std::optional<int> putts;
    ExternalFairwayResult fairway = ExternalFairwayResult::NotRecorded;
    std::optional<bool> greenInRegulation;
};

struct LaunchMetrics {
    std::optional<MetricValue> carryDistance;
    std::optional<MetricValue> totalDistance;
    std::optional<MetricValue> ballSpeed;
    std::optional<MetricValue> clubSpeed;
    std::optional<MetricValue> launchAngle;
    std::optional<MetricValue> launchDirection;
    std::optional<MetricValue> spinRate;
    std::optional<MetricValue> spinAxis;
    std::optional<MetricValue> peakHeight;
};

struct AdditionalMetric {
    std::string key;
    MetricValue value;
};

struct WeatherSnapshot {
    std::optional<MetricValue> temperature;
    std::optional<MetricValue> windSpeed;
    std::optional<MetricValue> windDirection;
    std::string conditionCode;
    std::string sourceText;
    Provenance provenance;
};

struct ExternalShot {
    int hole = 0;
    int sequence = 1;
    std::string club;
    std::string shotType;
    std::optional<Wgs84Position> start;
    std::optional<Wgs84Position> end;
    std::optional<MetricValue> distance;
    std::optional<MetricValue> lateral;
    LaunchMetrics launch;
    std::vector<AdditionalMetric> additionalMetrics;
    Provenance provenance;
};

struct ExternalRound {
    std::string courseName;
    std::optional<std::chrono::system_clock::time_point> startedAt;
    std::vector<ExternalHoleScore> scores;
    std::vector<ExternalShot> shots;
    std::optional<WeatherSnapshot> weather;
    Provenance provenance;
};

struct ExternalPracticeSession {
    std::string title;
    std::string venueName;
    std::optional<std::chrono::system_clock::time_point> startedAt;
    std::vector<ExternalShot> shots;
    std::optional<WeatherSnapshot> weather;
    Provenance provenance;
};

struct ImportResult {
    std::vector<ExternalRound> rounds;
    std::vector<ExternalPracticeSession> practiceSessions;
    std::vector<std::string> warnings;
};

// This interface is a capability/DTO scaffold. No vendor network provider is
// implemented yet, and these operations must never be treated as authorization.
class ExternalGolfDataProvider {
  public:
    virtual ~ExternalGolfDataProvider() = default;
    [[nodiscard]] virtual std::string id() const = 0;
    [[nodiscard]] virtual ProviderOperations implementedOperations() const = 0;
};

class GolfFileImporter : public ExternalGolfDataProvider {
  public:
    [[nodiscard]] virtual ImportResult importFile(std::span<const std::byte> bytes,
                                                  std::string_view fileName) const = 0;
};

// Cloud authorization and synchronization belong in the future self-hosted
// service. Device implementations must never collect vendor passwords.

} // namespace opencaddie::integrations
