#pragma once

#include <QObject>
#include <QVariantList>

namespace opencaddie::connectivity {

class NetworkManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList networks READ networks NOTIFY networksChanged)
    Q_PROPERTY(QString connectedSsid READ connectedSsid NOTIFY connectionChanged)
    Q_PROPERTY(int connectedSignalStrength READ connectedSignalStrength
                   NOTIFY connectionChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(bool internetReachable READ internetReachable
                   NOTIFY internetReachableChanged)

public:
    using QObject::QObject;
    ~NetworkManager() override = default;

    [[nodiscard]] virtual QVariantList networks() const = 0;
    [[nodiscard]] virtual QString connectedSsid() const = 0;
    [[nodiscard]] virtual int connectedSignalStrength() const = 0;
    [[nodiscard]] virtual bool scanning() const = 0;
    [[nodiscard]] virtual bool internetReachable() const = 0;

    Q_INVOKABLE virtual void scan() = 0;
    Q_INVOKABLE virtual void connectNetwork(const QString& ssid,
                                            const QString& password,
                                            bool hidden) = 0;
    Q_INVOKABLE virtual void forgetNetwork(const QString& ssid) = 0;

signals:
    void networksChanged();
    void connectionChanged();
    void scanningChanged();
    void internetReachableChanged();
    void operationFinished(bool success, const QString& message);
};

} // namespace opencaddie::connectivity
