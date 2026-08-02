#pragma once

#include "connectivity/BluetoothProvider.h"

#include <QTimer>

namespace opencaddie::connectivity {

class BluezBluetoothProvider final : public BluetoothProvider {
    Q_OBJECT

public:
    explicit BluezBluetoothProvider(QObject *parent = nullptr);
    [[nodiscard]] bool connected() const override;
    [[nodiscard]] QString deviceName() const override;

private:
    void refresh();

    bool m_connected = false;
    QString m_deviceName;
    QTimer m_refreshTimer;
};

} // namespace opencaddie::connectivity
