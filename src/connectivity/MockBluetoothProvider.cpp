#include "connectivity/MockBluetoothProvider.h"

namespace opencaddie::connectivity {

MockBluetoothProvider::MockBluetoothProvider(const bool connected,
                                             QObject *parent)
    : BluetoothProvider(parent), m_connected(connected) {}

bool MockBluetoothProvider::connected() const { return m_connected; }
QString MockBluetoothProvider::deviceName() const { return m_deviceName; }

void MockBluetoothProvider::setConnected(const bool connected,
                                         const QString &deviceName) {
    const QString resolvedName = connected ? deviceName : QString{};
    if (connected == m_connected && resolvedName == m_deviceName) return;
    m_connected = connected;
    m_deviceName = resolvedName;
    emit statusChanged();
}

} // namespace opencaddie::connectivity
