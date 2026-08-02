#include "domain/NearGreenTrigger.h"

#include <algorithm>

namespace opencaddie::domain {

NearGreenTrigger::NearGreenTrigger(const double entryMetres,
                                   const double exitMetres)
    : m_entryMetres(std::max(0.0, entryMetres)),
      m_exitMetres(std::max(m_entryMetres, exitMetres)) {}

void NearGreenTrigger::reset(const int hole) {
    m_hole = hole;
    m_armed = true;
}

bool NearGreenTrigger::update(const int hole, const double distanceMetres,
                              const bool usableFix, const bool scored) {
    if (hole != m_hole) reset(hole);
    if (scored) {
        m_armed = false;
        return false;
    }
    if (!usableFix) return false;
    if (distanceMetres > m_exitMetres) {
        m_armed = true;
        return false;
    }
    if (distanceMetres <= m_entryMetres && m_armed) {
        m_armed = false;
        return true;
    }
    return false;
}

bool NearGreenTrigger::armed() const { return m_armed; }

} // namespace opencaddie::domain
