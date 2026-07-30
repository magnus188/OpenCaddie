#pragma once

#include <QObject>

namespace opencaddie::platform {

class PowerProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(int batteryPercent READ batteryPercent NOTIFY powerChanged)
    Q_PROPERTY(bool externalPower READ externalPower NOTIFY powerChanged)
    Q_PROPERTY(int brightness READ brightness WRITE setBrightness
                   NOTIFY brightnessChanged)

public:
    using QObject::QObject;
    ~PowerProvider() override = default;
    [[nodiscard]] virtual int batteryPercent() const = 0;
    [[nodiscard]] virtual bool externalPower() const = 0;
    [[nodiscard]] virtual int brightness() const = 0;
    virtual void setBrightness(int percent) = 0;

signals:
    void powerChanged();
    void brightnessChanged();
};

} // namespace opencaddie::platform

