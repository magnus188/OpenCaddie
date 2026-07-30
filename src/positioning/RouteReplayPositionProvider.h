#pragma once

#include "positioning/PositionProvider.h"

#include <QElapsedTimer>
#include <QTimer>

#include <vector>

namespace opencaddie::positioning {

class RouteReplayPositionProvider final : public PositionProvider {
    Q_OBJECT
    Q_PROPERTY(double speed READ speed WRITE setSpeed NOTIFY speedChanged)

public:
    explicit RouteReplayPositionProvider(QString csvPath,
                                         QObject* parent = nullptr);
    void start() override;
    void stop() override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] double speed() const;
    void setSpeed(double speed);

signals:
    void speedChanged();

private:
    struct Sample {
        qint64 offsetMilliseconds;
        domain::GeoPoint point;
        double accuracyMetres;
    };

    bool load();
    void tick();

    QString m_csvPath;
    std::vector<Sample> m_samples;
    QTimer m_timer;
    QElapsedTimer m_elapsed;
    std::size_t m_index = 0;
    double m_speed = 1.0;
};

} // namespace opencaddie::positioning

