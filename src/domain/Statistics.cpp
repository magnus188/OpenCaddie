#include "domain/Statistics.h"

#include <cmath>

namespace opencaddie::domain {

HoleOutcome classifyHole(const int par, const int strokes) {
    if (par < 3 || par > 6 || strokes < 1) {
        return HoleOutcome::NotRecorded;
    }
    switch (strokes - par) {
    case -2:
        return HoleOutcome::Eagle;
    case -1:
        return HoleOutcome::Birdie;
    case 0:
        return HoleOutcome::Par;
    case 1:
        return HoleOutcome::Bogey;
    default:
        return strokes - par <= -3 ? HoleOutcome::AlbatrossOrBetter
                                   : HoleOutcome::DoubleBogeyOrWorse;
    }
}

std::string_view outcomeKey(const HoleOutcome outcome) {
    switch (outcome) {
    case HoleOutcome::AlbatrossOrBetter:
        return "albatross";
    case HoleOutcome::Eagle:
        return "eagle";
    case HoleOutcome::Birdie:
        return "birdie";
    case HoleOutcome::Par:
        return "par";
    case HoleOutcome::Bogey:
        return "bogey";
    case HoleOutcome::DoubleBogeyOrWorse:
        return "double_or_worse";
    case HoleOutcome::NotRecorded:
        return "not_recorded";
    }
    return "not_recorded";
}

double consistencyStandardDeviation(const std::span<const int> roundScoresToPar) {
    if (roundScoresToPar.size() < 2) {
        return 0.0;
    }
    double sum = 0.0;
    for (const int score : roundScoresToPar) {
        sum += score;
    }
    const double mean = sum / static_cast<double>(roundScoresToPar.size());
    double squaredDifference = 0.0;
    for (const int score : roundScoresToPar) {
        const double difference = static_cast<double>(score) - mean;
        squaredDifference += difference * difference;
    }
    return std::sqrt(squaredDifference / static_cast<double>(roundScoresToPar.size()));
}

} // namespace opencaddie::domain
