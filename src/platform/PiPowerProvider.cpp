#include "platform/PiPowerProvider.h"

#include <QFile>
#include <QTextStream>

#include <algorithm>

namespace opencaddie::platform {

PiPowerProvider::PiPowerProvider(QString brightnessPath, QObject* parent)
    : PowerProvider(parent), m_brightnessPath(std::move(brightnessPath)) {}

int PiPowerProvider::batteryPercent() const {
    // Raspberry Pi 5 has no generic battery telemetry. The production CM0
    // carrier will replace this provider with its fuel-gauge implementation.
    return -1;
}

bool PiPowerProvider::externalPower() const { return true; }
int PiPowerProvider::brightness() const { return m_brightness; }

void PiPowerProvider::setBrightness(const int percent) {
    const int bounded = std::clamp(percent, 10, 100);
    QFile file(m_brightnessPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream(&file) << qRound(255.0 * bounded / 100.0);
    }
    if (m_brightness != bounded) {
        m_brightness = bounded;
        emit brightnessChanged();
    }
}

} // namespace opencaddie::platform
