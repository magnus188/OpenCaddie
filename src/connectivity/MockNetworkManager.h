#pragma once

#include "connectivity/NetworkManager.h"

namespace opencaddie::connectivity {

class MockNetworkManager final : public NetworkManager {
    Q_OBJECT

public:
    using NetworkManager::NetworkManager;
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
    QVariantList m_networks;
    QString m_connectedSsid;
    int m_connectedSignalStrength = -1;
    bool m_scanning = false;
};

} // namespace opencaddie::connectivity
