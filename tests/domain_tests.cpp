#include "domain/ClubRecommendation.h"
#include "domain/Geo.h"
#include "domain/HoleSelector.h"
#include "domain/NearGreenTrigger.h"
#include "domain/PlaysLike.h"
#include "domain/Scoring.h"
#include "domain/Settings.h"
#include "domain/Statistics.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

namespace {
int failures = 0;

void check(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void checkNear(const double actual, const double expected, const double tolerance,
               const std::string_view message) {
    check(std::abs(actual - expected) <= tolerance, message);
}
} // namespace

int main() {
    using namespace opencaddie::domain;

    check(handicapStrokesForHole(20, 1) == 2,
          "20 handicap receives two strokes on SI 1");
    check(handicapStrokesForHole(20, 3) == 1,
          "20 handicap receives one stroke on SI 3");
    check(handicapStrokesForHole(-1, 18) == -1,
          "plus one gives a stroke back on SI 18");
    check(handicapStrokesForHole(-1, 1) == 0,
          "plus one does not give a stroke back on SI 1");
    check(stablefordPoints({1, 4, 1}, 5, 18) == 2, "net par is two Stableford points");
    check(stablefordPoints({1, 4, 18}, 3, 18) == 4,
          "net eagle is four Stableford points");

    const std::vector<HoleDefinition> holes{{1, 4, 1}, {2, 3, 2}};
    const std::vector<HoleScore> scores{{.hole = 1, .strokes = 5},
                                        {.hole = 2, .strokes = 3}};
    const auto summary = summarizeScores(holes, scores, 0);
    check(summary.gross == 8 && summary.versusPar == 1, "gross and versus-par summary");
    check(canUseHandicapScoring(holes), "valid par/index enables handicap");
    check(!canUseHandicapScoring(std::vector<HoleDefinition>{{1, 4, 0}, {2, 3, 2}}),
          "missing index blocks handicap");
    const std::vector<HoleDefinition> frontNine{
        {1, 4, 11}, {2, 4, 3}, {3, 3, 17}, {4, 5, 1},  {5, 4, 9},
        {6, 3, 15}, {7, 4, 5}, {8, 5, 7},  {9, 4, 13},
    };
    check(canUseHandicapScoring(frontNine),
          "an 18-hole course index remains valid for a nine-hole round");
    check(handicapIndexScale(frontNine) == 18,
          "nine-hole section preserves the original 18-hole index scale");
    check(stablefordPoints(frontNine.front(), 5, 18, handicapIndexScale(frontNine)) ==
              2,
          "nine-hole section allocates handicap on the 18-hole scale");
    const std::vector<HoleDefinition> nineHoleCourse{
        {1, 4, 1}, {2, 4, 2}, {3, 3, 3}, {4, 5, 4}, {5, 4, 5},
        {6, 3, 6}, {7, 4, 7}, {8, 5, 8}, {9, 4, 9},
    };
    check(handicapIndexScale(nineHoleCourse) == 9,
          "a true nine-hole course uses a 1-9 index scale");
    check(!canUseHandicapScoring(std::vector<HoleDefinition>{{1, 4, 1}, {2, 3, 1}}),
          "duplicate stroke indexes block handicap scoring");
    check(classifyHole(5, 2) == HoleOutcome::AlbatrossOrBetter,
          "three under is classified as albatross or better");
    check(classifyHole(4, 2) == HoleOutcome::Eagle, "two under is classified as eagle");
    check(classifyHole(4, 3) == HoleOutcome::Birdie,
          "one under is classified as birdie");
    check(classifyHole(4, 4) == HoleOutcome::Par, "level par is classified as par");
    check(classifyHole(4, 5) == HoleOutcome::Bogey, "one over is classified as bogey");
    check(classifyHole(2, 2) == HoleOutcome::NotRecorded &&
              classifyHole(7, 7) == HoleOutcome::NotRecorded,
          "out-of-range par is not classified as a scoring outcome");
    checkNear(consistencyStandardDeviation(std::vector<int>{4, 6, 8}), 1.633, 0.001,
              "consistency uses population standard deviation");

    const std::vector<Club> clubs{
        {"7i", "7 iron", 145.0, true, 0},
        {"6i", "6 iron", 158.0, true, 1},
        {"5i", "5 iron", 172.0, false, 2},
    };
    const auto advice = recommendClub(clubs, 154.0);
    check(advice && advice->club.id == "6i", "closest enabled club wins");
    checkNear(advice->deltaMetres, 4.0, 0.001, "club delta is shown");
    check(recommendClub(clubs, -1.0) == std::nullopt,
          "invalid target has no recommendation");

    const GeoPoint oslo{59.9139, 10.7522};
    const GeoPoint nearby{59.9148, 10.7522};
    checkNear(haversineMetres(oslo, nearby), 100.1, 1.0, "haversine distance");
    const LocalProjection projection{oslo, 6'378'137.0, -0.7};
    const auto [x, y] = projectToLocal(nearby, projection);
    const auto roundTrip = unprojectFromLocal(x, y, projection);
    checkNear(roundTrip.latitude, nearby.latitude, 1e-10,
              "projection latitude round-trip");
    checkNear(roundTrip.longitude, nearby.longitude, 1e-10,
              "projection longitude round-trip");

    const auto now = std::chrono::system_clock::now();
    check(isUsableFix({oslo, 8.0, now, true}, now), "fresh accurate GPS fix is usable");
    check(!isUsableFix({oslo, 26.0, now, true}, now),
          "club advice suppressed above 25m");
    check(!isUsableFix({oslo, 8.0, now - std::chrono::seconds(11), true}, now),
          "stale GPS fix is suppressed");

    HoleSelector selector(1, 3, 35.0);
    const std::vector<HoleProximity> closer{{1, 120.0}, {2, 20.0}};
    check(selector.update(closer) == 1, "hysteresis waits for confirmation 1");
    check(selector.update(closer) == 1, "hysteresis waits for confirmation 2");
    check(selector.update(closer) == 2, "hysteresis switches after confirmation");

    NearGreenTrigger scoreTrigger;
    check(!scoreTrigger.update(1, 34.0, false, false),
          "near-green prompt requires a usable fix");
    check(!scoreTrigger.update(1, 36.0, true, false),
          "near-green prompt waits for the entry threshold");
    check(scoreTrigger.update(1, 35.0, true, false),
          "near-green prompt fires at 35 metres");
    check(!scoreTrigger.update(1, 20.0, true, false),
          "near-green prompt fires once while inside the threshold");
    check(!scoreTrigger.update(1, 50.0, true, false),
          "near-green prompt does not re-arm at the exit boundary");
    check(!scoreTrigger.update(1, 51.0, true, false) && scoreTrigger.armed(),
          "near-green prompt re-arms beyond 50 metres");
    check(scoreTrigger.update(1, 30.0, true, false),
          "re-armed near-green prompt can fire after exiting");
    check(!scoreTrigger.update(1, 60.0, true, true) && !scoreTrigger.armed(),
          "saved score permanently suppresses the current hole");
    check(scoreTrigger.update(2, 25.0, true, false),
          "manual hole changes reset near-green trigger state");

    const GeoPoint bearingOrigin{59.0, 10.0};
    checkNear(initialBearingDegrees(bearingOrigin, {59.01, 10.0}), 0.0, 0.1,
              "bearing due north is zero");
    checkNear(initialBearingDegrees(bearingOrigin, {59.0, 10.01}), 90.0, 0.5,
              "bearing due east is ninety degrees");
    checkNear(initialBearingDegrees(bearingOrigin, {58.99, 10.0}), 180.0, 0.1,
              "bearing due south is one-eighty");
    checkNear(initialBearingDegrees(bearingOrigin, {59.0, 9.99}), 270.0, 0.5,
              "bearing due west is two-seventy");

    const WeatherConditions calm{};
    checkNear(computePlaysLike(150.0, 0.0, calm).totalMetres(), 150.0, 0.001,
              "no weather leaves the distance unchanged");
    check(!computePlaysLike(150.0, 0.0, calm).hasAdjustments(),
          "no weather reports no adjustments");

    WeatherConditions windy;
    windy.windSpeedMps = 5.0;
    windy.windFromDegrees = 0;
    const auto headwind = computePlaysLike(200.0, 0.0, windy);
    check(headwind.windMetres > 0.0, "headwind plays longer");
    checkNear(headwind.windMetres, 200.0 * 5.0 * 0.010, 0.01,
              "headwind magnitude follows the factor");
    windy.windFromDegrees = 180;
    const auto tailwind = computePlaysLike(200.0, 0.0, windy);
    check(tailwind.windMetres < 0.0, "tailwind plays shorter");
    checkNear(tailwind.windMetres, -headwind.windMetres / 2.0, 0.01,
              "tailwind helps half as much as headwind hurts");
    windy.windFromDegrees = 90;
    checkNear(computePlaysLike(200.0, 0.0, windy).windMetres, 0.0, 0.01,
              "pure crosswind has no distance effect");

    WeatherConditions cold;
    cold.temperatureC = 0.0;
    checkNear(computePlaysLike(200.0, 0.0, cold).temperatureMetres,
              200.0 * 20.0 * 0.0013, 0.01, "cold air plays longer");
    cold.temperatureC = 30.0;
    check(computePlaysLike(200.0, 0.0, cold).temperatureMetres < 0.0,
          "warm air plays shorter");

    WeatherConditions wet;
    wet.condition = "rain";
    checkNear(computePlaysLike(200.0, 0.0, wet).conditionMetres, 3.0, 0.01,
              "rain adds a fixed share of the base distance");
    wet.condition = "partly_cloudy";
    checkNear(computePlaysLike(200.0, 0.0, wet).conditionMetres, 0.0, 0.001,
              "dry conditions add nothing");

    WeatherConditions gale;
    gale.windSpeedMps = 60.0;
    gale.windFromDegrees = 180;
    check(computePlaysLike(50.0, 0.0, gale).totalMetres() >= 0.0,
          "plays-like distance never goes negative");

    Settings settings;
    check(!validateSettings(settings), "default settings are valid");
    settings.openGolfMapServer = "http://public.example";
    check(validateSettings(settings).has_value(),
          "public plain HTTP server is rejected");

    if (failures == 0) {
        std::cout << "All domain tests passed\n";
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
