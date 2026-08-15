#include "positioning/RouteReplayPositionProvider.h"

#include <QFile>
#include <QTextStream>

#include <algorithm>

namespace opencaddie::positioning {

RouteReplayPositionProvider::RouteReplayPositionProvider(QString csvPath,
                                                         QObject* parent)
    : PositionProvider(parent), m_csvPath(std::move(csvPath)) {
    m_timer.setInterval(100);
    connect(&m_timer, &QTimer::timeout, this,
            &RouteReplayPositionProvider::tick);
}

void RouteReplayPositionProvider::start() {
    if (m_samples.empty() && !load())
        return;
    m_index = 0;
    m_elapsed.restart();
    m_timer.start();
}

void RouteReplayPositionProvider::stop() { m_timer.stop(); }

QString RouteReplayPositionProvider::name() const {
    return QStringLiteral("route-replay");
}

double RouteReplayPositionProvider::speed() const { return m_speed; }

void RouteReplayPositionProvider::setSpeed(const double speed) {
    const double bounded = std::clamp(speed, 0.25, 20.0);
    if (qFuzzyCompare(m_speed, bounded)) return;
    m_speed = bounded;
    emit speedChanged();
}

bool RouteReplayPositionProvider::load() {
    QFile file(m_csvPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#') ||
            line.startsWith(QStringLiteral("offset_ms"))) {
            continue;
        }
        const auto columns = line.split(',');
        if (columns.size() < 4) continue;
        bool offsetOk = false;
        bool latitudeOk = false;
        bool longitudeOk = false;
        bool accuracyOk = false;
        const qint64 offset = columns[0].toLongLong(&offsetOk);
        const double latitude = columns[1].toDouble(&latitudeOk);
        const double longitude = columns[2].toDouble(&longitudeOk);
        const double accuracy = columns[3].toDouble(&accuracyOk);
        if (offsetOk && latitudeOk && longitudeOk && accuracyOk) {
            m_samples.push_back(
                {offset, {latitude, longitude}, accuracy});
        }
    }
    std::ranges::sort(m_samples, {}, &Sample::offsetMilliseconds);
    return !m_samples.empty();
}

void RouteReplayPositionProvider::tick() {
    if (m_index >= m_samples.size()) {
        m_index = 0;
        m_elapsed.restart();
    }
    const qint64 simulatedMilliseconds =
        static_cast<qint64>(static_cast<double>(m_elapsed.elapsed()) * m_speed);
    while (m_index < m_samples.size() &&
           m_samples[m_index].offsetMilliseconds <= simulatedMilliseconds) {
        const auto& sample = m_samples[m_index++];
        emit positionChanged({
            sample.point,
            sample.accuracyMetres,
            std::chrono::system_clock::now(),
            true,
        });
    }
}

} // namespace opencaddie::positioning
