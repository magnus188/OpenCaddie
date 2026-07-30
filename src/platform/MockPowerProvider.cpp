#include "platform/MockPowerProvider.h"

#include <algorithm>

namespace opencaddie::platform {

int MockPowerProvider::batteryPercent() const { return 76; }
bool MockPowerProvider::externalPower() const { return false; }
int MockPowerProvider::brightness() const { return m_brightness; }

void MockPowerProvider::setBrightness(const int percent) {
    const int bounded = std::clamp(percent, 10, 100);
    if (m_brightness == bounded) return;
    m_brightness = bounded;
    emit brightnessChanged();
}

} // namespace opencaddie::platform

