#pragma once

#include "connectivity/BluetoothProvider.h"

namespace opencaddie::connectivity {

class MockBluetoothProvider final : public BluetoothProvider {
    Q_OBJECT

public:
    explicit MockBluetoothProvider(bool connected = false,
                                   QObject *parent = nullptr);
    [[nodiscard]] bool connected() const override;
    [[nodiscard]] QString deviceName() const override;
    Q_INVOKABLE void setConnected(bool connected,
                                  const QString &deviceName = {});

private:
    bool m_connected = false;
    QString m_deviceName;
};

} // namespace opencaddie::connectivity
