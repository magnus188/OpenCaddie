#pragma once

#include "positioning/PositionProvider.h"

#include <QTcpSocket>
#include <QTimer>

namespace opencaddie::positioning {

class GpsdPositionProvider final : public PositionProvider {
    Q_OBJECT

public:
    explicit GpsdPositionProvider(QString host = QStringLiteral("127.0.0.1"),
                                  quint16 port = 2947,
                                  QObject* parent = nullptr);
    void start() override;
    void stop() override;
    [[nodiscard]] QString name() const override;

private:
    void readMessages();
    void parseMessage(const QByteArray& line);
    void scheduleReconnect();

    QString m_host;
    quint16 m_port;
    QTcpSocket m_socket;
    QTimer m_reconnect;
    QByteArray m_buffer;
};

} // namespace opencaddie::positioning

