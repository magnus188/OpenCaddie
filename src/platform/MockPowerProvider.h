#pragma once

#include "platform/PowerProvider.h"

namespace opencaddie::platform {

class MockPowerProvider final : public PowerProvider {
    Q_OBJECT

public:
    using PowerProvider::PowerProvider;
    [[nodiscard]] int batteryPercent() const override;
    [[nodiscard]] bool externalPower() const override;
    [[nodiscard]] int brightness() const override;
    void setBrightness(int percent) override;

private:
    int m_brightness = 80;
};

} // namespace opencaddie::platform

