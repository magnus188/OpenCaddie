#pragma once

#include <QColor>
#include <QJsonObject>
#include <QQuickPaintedItem>
#include <QQmlEngine>
#include <QSvgRenderer>
#include <QTransform>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

namespace opencaddie::ui {

class CourseMapItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(CourseMap)
    Q_PROPERTY(QUrl modelSource READ modelSource WRITE setModelSource
                   NOTIFY modelSourceChanged)
    Q_PROPERTY(int hole READ hole WRITE setHole NOTIFY holeChanged)
    Q_PROPERTY(QVariantMap colors READ colors WRITE setColors NOTIFY colorsChanged)
    Q_PROPERTY(double playerX READ playerX WRITE setPlayerX NOTIFY playerChanged)
    Q_PROPERTY(double playerY READ playerY WRITE setPlayerY NOTIFY playerChanged)
    Q_PROPERTY(bool playerVisible READ playerVisible WRITE setPlayerVisible
                   NOTIFY playerChanged)
    Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY viewTransformChanged)
    Q_PROPERTY(double panX READ panX WRITE setPanX NOTIFY viewTransformChanged)
    Q_PROPERTY(double panY READ panY WRITE setPanY NOTIFY viewTransformChanged)
    Q_PROPERTY(double rotationDegrees READ rotationDegrees WRITE setRotationDegrees
                   NOTIFY viewTransformChanged)
    Q_PROPERTY(QVariantList measurementPoints READ measurementPoints WRITE
                   setMeasurementPoints NOTIFY measurementChanged)
    Q_PROPERTY(bool measurementFromPlayer READ measurementFromPlayer WRITE
                   setMeasurementFromPlayer NOTIFY measurementChanged)
    Q_PROPERTY(bool measurementToTarget READ measurementToTarget WRITE
                   setMeasurementToTarget NOTIFY measurementChanged)
    Q_PROPERTY(bool metric READ metric WRITE setMetric NOTIFY measurementChanged)
    Q_PROPERTY(double measuredDistanceMetres READ measuredDistanceMetres
                   NOTIFY measurementChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY statusChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY statusChanged)

public:
    explicit CourseMapItem(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;
    [[nodiscard]] QUrl modelSource() const;
    void setModelSource(const QUrl& source);
    [[nodiscard]] int hole() const;
    void setHole(int hole);
    [[nodiscard]] QVariantMap colors() const;
    void setColors(const QVariantMap& colors);
    [[nodiscard]] double playerX() const;
    void setPlayerX(double value);
    [[nodiscard]] double playerY() const;
    void setPlayerY(double value);
    [[nodiscard]] bool playerVisible() const;
    void setPlayerVisible(bool value);
    [[nodiscard]] double zoom() const;
    void setZoom(double value);
    [[nodiscard]] double panX() const;
    void setPanX(double value);
    [[nodiscard]] double panY() const;
    void setPanY(double value);
    [[nodiscard]] double rotationDegrees() const;
    void setRotationDegrees(double value);
    [[nodiscard]] QVariantList measurementPoints() const;
    void setMeasurementPoints(const QVariantList& points);
    [[nodiscard]] bool measurementFromPlayer() const;
    void setMeasurementFromPlayer(bool value);
    [[nodiscard]] bool measurementToTarget() const;
    void setMeasurementToTarget(bool value);
    [[nodiscard]] bool metric() const;
    void setMetric(bool value);
    [[nodiscard]] double measuredDistanceMetres() const;
    [[nodiscard]] bool ready() const;
    [[nodiscard]] QString errorText() const;

    Q_INVOKABLE QVariantMap mapPointAt(double itemX, double itemY) const;

signals:
    void modelSourceChanged();
    void holeChanged();
    void colorsChanged();
    void playerChanged();
    void viewTransformChanged();
    void measurementChanged();
    void statusChanged();
    void loadError(const QString& message);

private:
    void reload();
    void failLoad(const QString &message, const QString &path);
    QByteArray buildSvg(const QJsonObject& hole) const;
    QColor colorFor(const QString& kind) const;
    QTransform mapToItemTransform() const;
    QVector<QPointF> measurementPath() const;
    void paintMeasurements(QPainter* painter) const;
    void paintPlayer(QPainter* painter) const;

    QUrl m_modelSource;
    int m_hole = 1;
    QVariantMap m_colors;
    double m_playerX = 0.0;
    double m_playerY = 0.0;
    bool m_playerVisible = false;
    double m_zoom = 1.0;
    double m_panX = 0.0;
    double m_panY = 0.0;
    double m_rotationDegrees = 0.0;
    QVariantList m_measurementPoints;
    bool m_measurementFromPlayer = true;
    bool m_measurementToTarget = false;
    bool m_metric = true;
    QPointF m_target;
    bool m_targetVisible = false;
    QRectF m_viewBox{-50.0, -50.0, 100.0, 100.0};
    QSvgRenderer m_renderer;
    bool m_ready = false;
    QString m_errorText;
};

} // namespace opencaddie::ui
