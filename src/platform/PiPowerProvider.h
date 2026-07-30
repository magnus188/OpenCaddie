#pragma once

#include "platform/PowerProvider.h"

#include <QString>

namespace opencaddie::platform {

class PiPowerProvider final : public PowerProvider {
    Q_OBJECT

public:
    explicit PiPowerProvider(
        QString brightnessPath =
            QStringLiteral("/sys/class/backlight/10-0045/brightness"),
        QObject* parent = nullptr);
    [[nodiscard]] int batteryPercent() const override;
    [[nodiscard]] bool externalPower() const override;
    [[nodiscard]] int brightness() const override;
    void setBrightness(int percent) override;

private:
    QString m_brightnessPath;
    int m_brightness = 80;
};

} // namespace opencaddie::platform

