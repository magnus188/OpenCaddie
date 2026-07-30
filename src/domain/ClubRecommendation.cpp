#include "domain/ClubRecommendation.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace opencaddie::domain {

std::optional<ClubAdvice> recommendClub(const std::span<const Club> clubs,
                                       const double targetMetres,
                                       const double recommendationBiasMetres) {
    if (!std::isfinite(targetMetres) || targetMetres <= 0.0) {
        return std::nullopt;
    }

    const double adjustedTarget = targetMetres + recommendationBiasMetres;
    const Club* best = nullptr;
    double bestAbsoluteDelta = std::numeric_limits<double>::infinity();
    for (const auto& club : clubs) {
        if (!club.enabled || !std::isfinite(club.carryMetres) ||
            club.carryMetres <= 0.0) {
            continue;
        }
        const double absoluteDelta = std::abs(club.carryMetres - adjustedTarget);
        if (absoluteDelta < bestAbsoluteDelta ||
            (absoluteDelta == bestAbsoluteDelta && best != nullptr &&
             club.carryMetres > best->carryMetres)) {
            best = &club;
            bestAbsoluteDelta = absoluteDelta;
        }
    }
    if (best == nullptr) {
        return std::nullopt;
    }

    return ClubAdvice{*best, targetMetres, best->carryMetres - targetMetres};
}

} // namespace opencaddie::domain

