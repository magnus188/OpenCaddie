#pragma once

#include <optional>
#include <span>
#include <vector>

namespace opencaddie::domain {

struct HoleProximity {
    int hole = 1;
    double distanceMetres = 0.0;
};

class HoleSelector {
public:
    explicit HoleSelector(int initialHole = 1, int confirmationsRequired = 3,
                          double switchAdvantageMetres = 35.0);

    int update(std::span<const HoleProximity> candidates);
    void selectManually(int hole);
    [[nodiscard]] int currentHole() const noexcept;

private:
    int m_currentHole;
    int m_confirmationsRequired;
    double m_switchAdvantageMetres;
    std::optional<int> m_pendingHole;
    int m_pendingConfirmations = 0;
};

} // namespace opencaddie::domain

