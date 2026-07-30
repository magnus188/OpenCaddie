#include "connectivity/NetworkManagerDbus.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QDBusVariant>

namespace opencaddie::connectivity {
namespace {
constexpr auto Service = "org.freedesktop.NetworkManager";
constexpr auto Root = "/org/freedesktop/NetworkManager";
constexpr auto ManagerInterface = "org.freedesktop.NetworkManager";
constexpr auto DeviceWirelessInterface =
    "org.freedesktop.NetworkManager.Device.Wireless";
constexpr auto PropertiesInterface = "org.freedesktop.DBus.Properties";

QVariant dbusProperty(const QString& path, const QString& interface,
                      const QString& name) {
    QDBusInterface properties(Service, path, PropertiesInterface,
                              QDBusConnection::systemBus());
    const QDBusReply<QDBusVariant> reply =
        properties.call(QStringLiteral("Get"), interface, name);
    return reply.isValid() ? reply.value().variant() : QVariant{};
}

QString ssidString(const QVariant& value) {
    return QString::fromUtf8(value.toByteArray());
}
} // namespace

NetworkManagerDbus::NetworkManagerDbus(QObject* parent)
    : NetworkManager(parent) {
    m_refreshTimer.setInterval(5'000);
    connect(&m_refreshTimer, &QTimer::timeout, this,
            &NetworkManagerDbus::refresh);
    m_refreshTimer.start();
    refresh();
}

QVariantList NetworkManagerDbus::networks() const { return m_networks; }
QString NetworkManagerDbus::connectedSsid() const { return m_connectedSsid; }
bool NetworkManagerDbus::scanning() const { return m_scanning; }
bool NetworkManagerDbus::internetReachable() const {
    return m_internetReachable;
}

std::optional<QDBusObjectPath> NetworkManagerDbus::wirelessDevice() const {
    QDBusInterface manager(Service, Root, ManagerInterface,
                           QDBusConnection::systemBus());
    const QDBusReply<QList<QDBusObjectPath>> reply =
        manager.call(QStringLiteral("GetDevices"));
    if (!reply.isValid()) return std::nullopt;
    for (const auto& device : reply.value()) {
        const uint type =
            dbusProperty(device.path(), QStringLiteral("org.freedesktop.NetworkManager.Device"),
                         QStringLiteral("DeviceType"))
                .toUInt();
        if (type == 2) return device; // NM_DEVICE_TYPE_WIFI
    }
    return std::nullopt;
}

void NetworkManagerDbus::scan() {
    const auto device = wirelessDevice();
    if (!device) {
        emit operationFinished(false, tr("No Wi-Fi adapter was found."));
        return;
    }
    m_scanning = true;
    emit scanningChanged();
    QDBusInterface wireless(Service, device->path(), DeviceWirelessInterface,
                            QDBusConnection::systemBus());
    wireless.asyncCall(QStringLiteral("RequestScan"), QVariantMap{});
    QTimer::singleShot(2'000, this, [this] {
        m_scanning = false;
        emit scanningChanged();
        refresh();
    });
}

void NetworkManagerDbus::refresh() {
    const auto device = wirelessDevice();
    if (!device) return;
    QDBusInterface wireless(Service, device->path(), DeviceWirelessInterface,
                            QDBusConnection::systemBus());
    const QDBusReply<QList<QDBusObjectPath>> reply =
        wireless.call(QStringLiteral("GetAllAccessPoints"));
    if (!reply.isValid()) return;

    QVariantList networks;
    QSet<QString> seen;
    for (const auto& accessPoint : reply.value()) {
        const QString ssid = ssidString(
            dbusProperty(accessPoint.path(),
                         QStringLiteral("org.freedesktop.NetworkManager.AccessPoint"),
                         QStringLiteral("Ssid")));
        if (ssid.isEmpty() || seen.contains(ssid)) continue;
        seen.insert(ssid);
        const uint strength =
            dbusProperty(accessPoint.path(),
                         QStringLiteral("org.freedesktop.NetworkManager.AccessPoint"),
                         QStringLiteral("Strength"))
                .toUInt();
        const uint rsnFlags =
            dbusProperty(accessPoint.path(),
                         QStringLiteral("org.freedesktop.NetworkManager.AccessPoint"),
                         QStringLiteral("RsnFlags"))
                .toUInt();
        const uint wpaFlags =
            dbusProperty(accessPoint.path(),
                         QStringLiteral("org.freedesktop.NetworkManager.AccessPoint"),
                         QStringLiteral("WpaFlags"))
                .toUInt();
        networks.push_back(QVariantMap{
            {QStringLiteral("ssid"), ssid},
            {QStringLiteral("signal"), strength},
            {QStringLiteral("security"),
             rsnFlags != 0 ? QStringLiteral("WPA2/WPA3")
                           : wpaFlags != 0 ? QStringLiteral("WPA")
                                           : QStringLiteral("Open")},
            {QStringLiteral("saved"), false},
        });
    }
    m_networks = networks;
    emit networksChanged();

    const uint connectivity =
        dbusProperty(QString::fromLatin1(Root), QString::fromLatin1(ManagerInterface),
                     QStringLiteral("Connectivity"))
            .toUInt();
    const bool reachable = connectivity == 4; // NM_CONNECTIVITY_FULL
    if (reachable != m_internetReachable) {
        m_internetReachable = reachable;
        emit internetReachableChanged();
    }
}

std::optional<QDBusObjectPath>
NetworkManagerDbus::accessPointFor(const QString& ssid) const {
    const auto device = wirelessDevice();
    if (!device) return std::nullopt;
    QDBusInterface wireless(Service, device->path(), DeviceWirelessInterface,
                            QDBusConnection::systemBus());
    const QDBusReply<QList<QDBusObjectPath>> reply =
        wireless.call(QStringLiteral("GetAllAccessPoints"));
    if (!reply.isValid()) return std::nullopt;
    for (const auto& accessPoint : reply.value()) {
        if (ssidString(dbusProperty(
                accessPoint.path(),
                QStringLiteral("org.freedesktop.NetworkManager.AccessPoint"),
                QStringLiteral("Ssid"))) == ssid) {
            return accessPoint;
        }
    }
    return std::nullopt;
}

void NetworkManagerDbus::connectNetwork(const QString& ssid,
                                        const QString& password,
                                        const bool hidden) {
    const auto device = wirelessDevice();
    const auto accessPoint = accessPointFor(ssid);
    if (!device || (!accessPoint && !hidden)) {
        emit operationFinished(false, tr("The selected network is unavailable."));
        return;
    }

    QVariantMap connection{
        {QStringLiteral("id"), ssid},
        {QStringLiteral("type"), QStringLiteral("802-11-wireless")},
        {QStringLiteral("autoconnect"), true},
    };
    QVariantMap wireless{
        {QStringLiteral("ssid"), ssid.toUtf8()},
        {QStringLiteral("mode"), QStringLiteral("infrastructure")},
        {QStringLiteral("hidden"), hidden},
    };
    QVariantMap security;
    if (!password.isEmpty()) {
        wireless.insert(QStringLiteral("security"),
                        QStringLiteral("802-11-wireless-security"));
        security = {
            {QStringLiteral("key-mgmt"), QStringLiteral("wpa-psk")},
            {QStringLiteral("psk"), password},
        };
    }
    QVariantMap ipv4{{QStringLiteral("method"), QStringLiteral("auto")}};
    QVariantMap ipv6{{QStringLiteral("method"), QStringLiteral("auto")}};
    QVariantMap settings{
        {QStringLiteral("connection"), connection},
        {QStringLiteral("802-11-wireless"), wireless},
        {QStringLiteral("ipv4"), ipv4},
        {QStringLiteral("ipv6"), ipv6},
    };
    if (!security.isEmpty()) {
        settings.insert(QStringLiteral("802-11-wireless-security"), security);
    }

    QDBusInterface manager(Service, Root, ManagerInterface,
                           QDBusConnection::systemBus());
    const QDBusMessage reply = manager.call(
        QStringLiteral("AddAndActivateConnection"), settings,
        QVariant::fromValue(*device),
        QVariant::fromValue(accessPoint.value_or(QDBusObjectPath(QStringLiteral("/")))));
    if (reply.type() == QDBusMessage::ErrorMessage) {
        emit operationFinished(false, tr("Could not connect to the network."));
        return;
    }
    m_connectedSsid = ssid;
    emit connectionChanged();
    emit operationFinished(true, tr("Connected to %1").arg(ssid));
    // Passwords are intentionally never persisted or logged by OpenCaddie;
    // NetworkManager owns the connection secret from this point.
}

void NetworkManagerDbus::forgetNetwork(const QString& ssid) {
    QDBusInterface settings(Service,
                            QStringLiteral("/org/freedesktop/NetworkManager/Settings"),
                            QStringLiteral("org.freedesktop.NetworkManager.Settings"),
                            QDBusConnection::systemBus());
    const QDBusReply<QList<QDBusObjectPath>> reply =
        settings.call(QStringLiteral("ListConnections"));
    if (!reply.isValid()) {
        emit operationFinished(false, tr("Could not list saved networks."));
        return;
    }
    bool removed = false;
    for (const auto& path : reply.value()) {
        QDBusInterface connection(
            Service, path.path(),
            QStringLiteral("org.freedesktop.NetworkManager.Settings.Connection"),
            QDBusConnection::systemBus());
        const QDBusMessage settingsReply =
            connection.call(QStringLiteral("GetSettings"));
        if (settingsReply.arguments().isEmpty()) continue;
        const QVariantMap all = qdbus_cast<QVariantMap>(
            settingsReply.arguments().constFirst());
        const QString id =
            all.value(QStringLiteral("connection")).toMap().value(
                QStringLiteral("id")).toString();
        if (id == ssid) {
            removed =
                connection.call(QStringLiteral("Delete")).type() !=
                QDBusMessage::ErrorMessage;
        }
    }
    if (m_connectedSsid == ssid) {
        m_connectedSsid.clear();
        emit connectionChanged();
    }
    emit operationFinished(removed,
                           removed ? tr("Network forgotten.")
                                   : tr("Saved network was not found."));
}

} // namespace opencaddie::connectivity
