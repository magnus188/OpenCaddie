#include "connectivity/BluezBluetoothProvider.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

namespace opencaddie::connectivity {

BluezBluetoothProvider::BluezBluetoothProvider(QObject *parent)
    : BluetoothProvider(parent) {
    m_refreshTimer.setInterval(5'000);
    connect(&m_refreshTimer, &QTimer::timeout, this,
            &BluezBluetoothProvider::refresh);
    m_refreshTimer.start();
    refresh();
}

bool BluezBluetoothProvider::connected() const { return m_connected; }
QString BluezBluetoothProvider::deviceName() const { return m_deviceName; }

void BluezBluetoothProvider::refresh() {
    QDBusInterface manager(QStringLiteral("org.bluez"), QStringLiteral("/"),
                           QStringLiteral("org.freedesktop.DBus.ObjectManager"),
                           QDBusConnection::systemBus());
    const QDBusMessage reply = manager.call(QStringLiteral("GetManagedObjects"));
    bool connectedDeviceFound = false;
    QString connectedDeviceName;
    if (reply.type() != QDBusMessage::ErrorMessage &&
        !reply.arguments().isEmpty()) {
        QDBusArgument objects =
            reply.arguments().constFirst().value<QDBusArgument>();
        objects.beginMap();
        while (!objects.atEnd() && !connectedDeviceFound) {
            objects.beginMapEntry();
            QDBusObjectPath objectPath;
            objects >> objectPath;
            objects.beginMap();
            while (!objects.atEnd()) {
                objects.beginMapEntry();
                QString interfaceName;
                QVariantMap properties;
                objects >> interfaceName >> properties;
                objects.endMapEntry();
                if (interfaceName == QStringLiteral("org.bluez.Device1") &&
                    properties.value(QStringLiteral("Connected")).toBool()) {
                    connectedDeviceFound = true;
                    connectedDeviceName =
                        properties.value(QStringLiteral("Alias")).toString();
                    if (connectedDeviceName.isEmpty()) {
                        connectedDeviceName =
                            properties.value(QStringLiteral("Name")).toString();
                    }
                }
            }
            objects.endMap();
            objects.endMapEntry();
        }
        objects.endMap();
    }
    if (connectedDeviceFound == m_connected &&
        connectedDeviceName == m_deviceName) return;
    m_connected = connectedDeviceFound;
    m_deviceName = connectedDeviceName;
    emit statusChanged();
}

} // namespace opencaddie::connectivity
