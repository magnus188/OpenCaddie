#pragma once

#include "domain/Types.h"

#include <span>

namespace opencaddie::domain {

struct ScoreSummary {
    int gross = 0;
    int par = 0;
    int versusPar = 0;
    int stableford = 0;
};

int handicapIndexScale(std::span<const HoleDefinition> holes);
int handicapStrokesForHole(int courseHandicap, int strokeIndex,
                           int handicapIndexScale = 18);
int stablefordPoints(const HoleDefinition& hole, int grossStrokes, int courseHandicap,
                     int handicapIndexScale = 18);
ScoreSummary summarizeScores(std::span<const HoleDefinition> holes,
                             std::span<const HoleScore> scores,
                             int courseHandicap);
bool canUseHandicapScoring(std::span<const HoleDefinition> holes);

} // namespace opencaddie::domain
