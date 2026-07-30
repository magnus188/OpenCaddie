#include "domain/HoleSelector.h"

#include <algorithm>
#include <cmath>

namespace opencaddie::domain {

HoleSelector::HoleSelector(const int initialHole, const int confirmationsRequired,
                           const double switchAdvantageMetres)
    : m_currentHole(std::max(1, initialHole)),
      m_confirmationsRequired(std::max(1, confirmationsRequired)),
      m_switchAdvantageMetres(std::max(0.0, switchAdvantageMetres)) {}

int HoleSelector::update(const std::span<const HoleProximity> candidates) {
    if (candidates.empty()) {
        m_pendingHole.reset();
        m_pendingConfirmations = 0;
        return m_currentHole;
    }

    const auto nearest = std::ranges::min_element(
        candidates, {}, &HoleProximity::distanceMetres);
    const auto current = std::ranges::find(candidates, m_currentHole,
                                           &HoleProximity::hole);
    if (nearest == candidates.end() || !std::isfinite(nearest->distanceMetres) ||
        nearest->hole == m_currentHole) {
        m_pendingHole.reset();
        m_pendingConfirmations = 0;
        return m_currentHole;
    }

    const double currentDistance =
        current == candidates.end() ? std::numeric_limits<double>::infinity()
                                    : current->distanceMetres;
    if (nearest->distanceMetres + m_switchAdvantageMetres >= currentDistance) {
        m_pendingHole.reset();
        m_pendingConfirmations = 0;
        return m_currentHole;
    }

    if (m_pendingHole != nearest->hole) {
        m_pendingHole = nearest->hole;
        m_pendingConfirmations = 1;
    } else {
        ++m_pendingConfirmations;
    }
    if (m_pendingConfirmations >= m_confirmationsRequired) {
        m_currentHole = *m_pendingHole;
        m_pendingHole.reset();
        m_pendingConfirmations = 0;
    }
    return m_currentHole;
}

void HoleSelector::selectManually(const int hole) {
    m_currentHole = std::max(1, hole);
    m_pendingHole.reset();
    m_pendingConfirmations = 0;
}

int HoleSelector::currentHole() const noexcept { return m_currentHole; }

} // namespace opencaddie::domain

