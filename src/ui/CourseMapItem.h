#pragma once

#include <QColor>
#include <QJsonObject>
#include <QQuickPaintedItem>
#include <QQmlEngine>
#include <QSvgRenderer>
#include <QUrl>
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

signals:
    void modelSourceChanged();
    void holeChanged();
    void colorsChanged();
    void playerChanged();
    void loadError(const QString& message);

private:
    void reload();
    QByteArray buildSvg(const QJsonObject& hole) const;
    QColor colorFor(const QString& kind) const;
    QRectF renderedMapRect() const;

    QUrl m_modelSource;
    int m_hole = 1;
    QVariantMap m_colors;
    double m_playerX = 0.0;
    double m_playerY = 0.0;
    bool m_playerVisible = false;
    QRectF m_viewBox{-50.0, -50.0, 100.0, 100.0};
    QSvgRenderer m_renderer;
};

} // namespace opencaddie::ui
