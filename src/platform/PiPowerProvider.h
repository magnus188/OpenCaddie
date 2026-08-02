#pragma once

#include "platform/PowerProvider.h"

#include <QString>
#include <QTimer>

namespace opencaddie::platform {

class PiPowerProvider final : public PowerProvider {
    Q_OBJECT

public:
    explicit PiPowerProvider(
        QString brightnessPath =
            QStringLiteral("/sys/class/backlight/10-0045/brightness"),
        QString powerSupplyRoot = QStringLiteral("/sys/class/power_supply"),
        QObject* parent = nullptr);
    [[nodiscard]] int batteryPercent() const override;
    [[nodiscard]] bool externalPower() const override;
    [[nodiscard]] int brightness() const override;
    void setBrightness(int percent) override;

private:
    void refreshPower();

    QString m_brightnessPath;
    QString m_powerSupplyRoot;
    QTimer m_powerTimer;
    int m_batteryPercent = -1;
    bool m_externalPower = true;
    int m_brightness = 80;
};

} // namespace opencaddie::platform
