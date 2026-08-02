#pragma once

#include <span>
#include <string_view>

namespace opencaddie::domain {

enum class HoleOutcome {
    NotRecorded,
    AlbatrossOrBetter,
    Eagle,
    Birdie,
    Par,
    Bogey,
    DoubleBogeyOrWorse,
};

HoleOutcome classifyHole(int par, int strokes);
std::string_view outcomeKey(HoleOutcome outcome);
double consistencyStandardDeviation(std::span<const int> roundScoresToPar);

} // namespace opencaddie::domain
