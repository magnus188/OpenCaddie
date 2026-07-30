#include "positioning/GpsdPositionProvider.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>

namespace opencaddie::positioning {

GpsdPositionProvider::GpsdPositionProvider(QString host, const quint16 port,
                                           QObject* parent)
    : PositionProvider(parent), m_host(std::move(host)), m_port(port) {
    m_reconnect.setSingleShot(true);
    m_reconnect.setInterval(2'000);
    connect(&m_reconnect, &QTimer::timeout, this, &GpsdPositionProvider::start);
    connect(&m_socket, &QTcpSocket::connected, this, [this] {
        m_socket.write("?WATCH={\"enable\":true,\"json\":true};\n");
        emit diagnosticsChanged({
            {QStringLiteral("provider"), name()},
            {QStringLiteral("state"), QStringLiteral("connected")},
        });
    });
    connect(&m_socket, &QTcpSocket::readyRead, this,
            &GpsdPositionProvider::readMessages);
    connect(&m_socket, &QTcpSocket::disconnected, this,
            &GpsdPositionProvider::scheduleReconnect);
    connect(&m_socket, &QTcpSocket::errorOccurred, this,
            [this](QAbstractSocket::SocketError) {
                emit diagnosticsChanged({
                    {QStringLiteral("provider"), name()},
                    {QStringLiteral("state"), QStringLiteral("error")},
                    {QStringLiteral("message"), m_socket.errorString()},
                });
                scheduleReconnect();
            });
}

void GpsdPositionProvider::start() {
    if (m_socket.state() != QAbstractSocket::UnconnectedState) return;
    m_socket.connectToHost(m_host, m_port);
}

void GpsdPositionProvider::stop() {
    m_reconnect.stop();
    m_socket.abort();
}

QString GpsdPositionProvider::name() const { return QStringLiteral("gpsd"); }

void GpsdPositionProvider::readMessages() {
    m_buffer.append(m_socket.readAll());
    qsizetype newline = -1;
    while ((newline = m_buffer.indexOf('\n')) >= 0) {
        const QByteArray line = m_buffer.left(newline).trimmed();
        m_buffer.remove(0, newline + 1);
        if (!line.isEmpty()) parseMessage(line);
    }
}

void GpsdPositionProvider::parseMessage(const QByteArray& line) {
    const QJsonObject object = QJsonDocument::fromJson(line).object();
    if (object.value(QStringLiteral("class")).toString() !=
        QStringLiteral("TPV")) {
        return;
    }
    const int mode = object.value(QStringLiteral("mode")).toInt();
    const double latitude = object.value(QStringLiteral("lat")).toDouble(
        std::numeric_limits<double>::quiet_NaN());
    const double longitude = object.value(QStringLiteral("lon")).toDouble(
        std::numeric_limits<double>::quiet_NaN());
    const double epx = object.value(QStringLiteral("epx")).toDouble(99.0);
    const double epy = object.value(QStringLiteral("epy")).toDouble(99.0);
    const double accuracy = std::max(epx, epy);
    const auto timestamp =
        QDateTime::fromString(object.value(QStringLiteral("time")).toString(),
                              Qt::ISODateWithMs)
            .toUTC();

    domain::PositionFix fix;
    fix.point = {latitude, longitude};
    fix.accuracyMetres = accuracy;
    fix.timestamp = std::chrono::system_clock::time_point{
        std::chrono::milliseconds(timestamp.toMSecsSinceEpoch())};
    fix.valid = mode >= 2 && std::isfinite(latitude) &&
                std::isfinite(longitude) && timestamp.isValid();
    emit positionChanged(fix);
    emit diagnosticsChanged({
        {QStringLiteral("provider"), name()},
        {QStringLiteral("state"), QStringLiteral("fix")},
        {QStringLiteral("mode"), mode},
        {QStringLiteral("accuracyMetres"), accuracy},
    });
}

void GpsdPositionProvider::scheduleReconnect() {
    if (!m_reconnect.isActive()) m_reconnect.start();
}

} // namespace opencaddie::positioning

