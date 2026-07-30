#include "domain/Scoring.h"

#include <algorithm>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>

namespace opencaddie::domain {

int handicapIndexScale(const std::span<const HoleDefinition> holes) {
    if (holes.empty()) {
        return 0;
    }
    const auto maximumIndex = std::ranges::max_element(
        holes, {}, &HoleDefinition::strokeIndex);
    return holes.size() > 9 || maximumIndex->strokeIndex > 9 ? 18 : 9;
}

int handicapStrokesForHole(const int courseHandicap, const int strokeIndex,
                           const int handicapIndexScale) {
    if (handicapIndexScale <= 0 || strokeIndex < 1 ||
        strokeIndex > handicapIndexScale ||
        courseHandicap == 0) {
        return 0;
    }

    const int absoluteHandicap = std::abs(courseHandicap);
    const int base = absoluteHandicap / handicapIndexScale;
    const int remainder = absoluteHandicap % handicapIndexScale;
    if (courseHandicap > 0) {
        return base + (strokeIndex <= remainder ? 1 : 0);
    }
    return -(base +
             (strokeIndex > handicapIndexScale - remainder ? 1 : 0));
}

int stablefordPoints(const HoleDefinition& hole, const int grossStrokes,
                     const int courseHandicap,
                     const int handicapIndexScale) {
    if (grossStrokes <= 0 || hole.par < 1 || hole.strokeIndex < 1) {
        return 0;
    }
    const int received = handicapStrokesForHole(
        courseHandicap, hole.strokeIndex, handicapIndexScale);
    return std::max(0, 2 + hole.par + received - grossStrokes);
}

ScoreSummary summarizeScores(const std::span<const HoleDefinition> holes,
                             const std::span<const HoleScore> scores,
                             const int courseHandicap) {
    std::unordered_map<int, HoleDefinition> definitionByNumber;
    for (const auto& hole : holes) {
        definitionByNumber.emplace(hole.number, hole);
    }

    ScoreSummary summary;
    const int indexScale = handicapIndexScale(holes);
    for (const auto& score : scores) {
        if (score.strokes <= 0) {
            continue;
        }
        const auto definition = definitionByNumber.find(score.hole);
        if (definition == definitionByNumber.end()) {
            continue;
        }
        summary.gross += score.strokes;
        summary.par += definition->second.par;
        summary.stableford +=
            stablefordPoints(definition->second, score.strokes, courseHandicap,
                             indexScale);
    }
    summary.versusPar = summary.gross - summary.par;
    return summary;
}

bool canUseHandicapScoring(const std::span<const HoleDefinition> holes) {
    if (holes.empty()) {
        return false;
    }
    std::unordered_set<int> indexes;
    return std::ranges::all_of(holes, [&indexes](const auto& hole) {
        return hole.par >= 3 && hole.par <= 6 && hole.strokeIndex >= 1 &&
               hole.strokeIndex <= 18 &&
               indexes.insert(hole.strokeIndex).second;
    });
}

} // namespace opencaddie::domain
