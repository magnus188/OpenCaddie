#pragma once

#include <QObject>

namespace opencaddie::connectivity {

class BluetoothProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY statusChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY statusChanged)

public:
    using QObject::QObject;
    ~BluetoothProvider() override = default;

    [[nodiscard]] virtual bool connected() const = 0;
    [[nodiscard]] virtual QString deviceName() const = 0;

signals:
    void statusChanged();
};

} // namespace opencaddie::connectivity
