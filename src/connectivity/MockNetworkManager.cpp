#include "connectivity/MockNetworkManager.h"

#include <QTimer>

namespace opencaddie::connectivity {

QVariantList MockNetworkManager::networks() const { return m_networks; }
QString MockNetworkManager::connectedSsid() const { return m_connectedSsid; }
int MockNetworkManager::connectedSignalStrength() const {
    return m_connectedSignalStrength;
}
bool MockNetworkManager::scanning() const { return m_scanning; }
bool MockNetworkManager::internetReachable() const {
    return !m_connectedSsid.isEmpty();
}

void MockNetworkManager::scan() {
    if (m_scanning) return;
    m_scanning = true;
    emit scanningChanged();
    QTimer::singleShot(500, this, [this] {
        m_networks = {
            QVariantMap{{QStringLiteral("ssid"), QStringLiteral("Clubhouse")},
                        {QStringLiteral("signal"), 84},
                        {QStringLiteral("security"), QStringLiteral("WPA2/WPA3")},
                        {QStringLiteral("saved"), true}},
            QVariantMap{{QStringLiteral("ssid"), QStringLiteral("OpenCaddie Lab")},
                        {QStringLiteral("signal"), 61},
                        {QStringLiteral("security"), QStringLiteral("WPA2")},
                        {QStringLiteral("saved"), false}},
            QVariantMap{{QStringLiteral("ssid"), QStringLiteral("Guest")},
                        {QStringLiteral("signal"), 42},
                        {QStringLiteral("security"), QStringLiteral("Open")},
                        {QStringLiteral("saved"), false}},
        };
        m_scanning = false;
        emit networksChanged();
        emit scanningChanged();
    });
}

void MockNetworkManager::connectNetwork(const QString& ssid,
                                        const QString& password,
                                        const bool) {
    // The mock only checks whether a protected demo network has a password.
    if (ssid != QStringLiteral("Guest") && password.isEmpty()) {
        emit operationFinished(false, tr("Password is required."));
        return;
    }
    m_connectedSsid = ssid;
    m_connectedSignalStrength = -1;
    for (const auto &network : std::as_const(m_networks)) {
        const auto values = network.toMap();
        if (values.value(QStringLiteral("ssid")).toString() == ssid) {
            m_connectedSignalStrength =
                values.value(QStringLiteral("signal"), -1).toInt();
            break;
        }
    }
    emit connectionChanged();
    emit internetReachableChanged();
    emit operationFinished(true, tr("Connected to %1").arg(ssid));
}

void MockNetworkManager::forgetNetwork(const QString& ssid) {
    if (m_connectedSsid == ssid) {
        m_connectedSsid.clear();
        m_connectedSignalStrength = -1;
        emit connectionChanged();
        emit internetReachableChanged();
    }
    emit operationFinished(true, tr("Network forgotten."));
}

} // namespace opencaddie::connectivity
