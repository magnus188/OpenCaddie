#pragma once

#include "connectivity/NetworkManager.h"

#include <QDBusObjectPath>
#include <QTimer>

namespace opencaddie::connectivity {

class NetworkManagerDbus final : public NetworkManager {
    Q_OBJECT

public:
    explicit NetworkManagerDbus(QObject* parent = nullptr);
    [[nodiscard]] QVariantList networks() const override;
    [[nodiscard]] QString connectedSsid() const override;
    [[nodiscard]] int connectedSignalStrength() const override;
    [[nodiscard]] bool scanning() const override;
    [[nodiscard]] bool internetReachable() const override;
    void scan() override;
    void connectNetwork(const QString& ssid, const QString& password,
                        bool hidden) override;
    void forgetNetwork(const QString& ssid) override;

private:
    void refresh();
    std::optional<QDBusObjectPath> wirelessDevice() const;
    std::optional<QDBusObjectPath> accessPointFor(const QString& ssid) const;

    QVariantList m_networks;
    QString m_connectedSsid;
    int m_connectedSignalStrength = -1;
    bool m_scanning = false;
    bool m_internetReachable = false;
    QTimer m_refreshTimer;
};

} // namespace opencaddie::connectivity
