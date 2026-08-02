#include "ui/CourseMapItem.h"

#include <QFile>
#include <QFont>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLoggingCategory>
#include <QPainterPath>
#include <QPainter>
#include <QXmlStreamWriter>

#include <algorithm>
#include <cmath>

namespace opencaddie::ui {

CourseMapItem::CourseMapItem(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true);
    m_colors = {
        {QStringLiteral("rough"), QStringLiteral("#315C35")},
        {QStringLiteral("fairway"), QStringLiteral("#2FCB63")},
        {QStringLiteral("green"), QStringLiteral("#8ED66B")},
        {QStringLiteral("tee"), QStringLiteral("#70B85B")},
        {QStringLiteral("bunker"), QStringLiteral("#E0C27A")},
        {QStringLiteral("water"), QStringLiteral("#2BA7D7")},
        {QStringLiteral("wood"), QStringLiteral("#1A5B35")},
        {QStringLiteral("path"), QStringLiteral("#8B8174")},
        {QStringLiteral("hole_line"), QStringLiteral("#F7F8F2")},
        {QStringLiteral("pin"), QStringLiteral("#D94D3E")},
    };
}

void CourseMapItem::paint(QPainter* painter) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    if (m_ready && m_renderer.isValid()) {
        painter->save();
        painter->setClipRect(boundingRect());
        painter->setTransform(mapToItemTransform(), true);
        m_renderer.render(painter, m_viewBox);
        painter->restore();
    }
    paintMeasurements(painter);
    paintPlayer(painter);
}

QUrl CourseMapItem::modelSource() const { return m_modelSource; }
int CourseMapItem::hole() const { return m_hole; }
QVariantMap CourseMapItem::colors() const { return m_colors; }
double CourseMapItem::playerX() const { return m_playerX; }
double CourseMapItem::playerY() const { return m_playerY; }
bool CourseMapItem::playerVisible() const { return m_playerVisible; }
double CourseMapItem::zoom() const { return m_zoom; }
double CourseMapItem::panX() const { return m_panX; }
double CourseMapItem::panY() const { return m_panY; }
double CourseMapItem::rotationDegrees() const { return m_rotationDegrees; }
QVariantList CourseMapItem::measurementPoints() const {
    return m_measurementPoints;
}
bool CourseMapItem::measurementFromPlayer() const {
    return m_measurementFromPlayer;
}
bool CourseMapItem::measurementToTarget() const {
    return m_measurementToTarget;
}
bool CourseMapItem::metric() const { return m_metric; }

void CourseMapItem::setModelSource(const QUrl& source) {
    if (source == m_modelSource) return;
    m_modelSource = source;
    emit modelSourceChanged();
    reload();
}

void CourseMapItem::setHole(const int hole) {
    if (m_hole == hole) return;
    m_hole = hole;
    emit holeChanged();
    reload();
}

void CourseMapItem::setColors(const QVariantMap& colors) {
    if (m_colors == colors) return;
    for (auto iterator = colors.cbegin(); iterator != colors.cend(); ++iterator) {
        if (QColor(iterator.value().toString()).isValid()) {
            m_colors.insert(iterator.key(), iterator.value());
        }
    }
    emit colorsChanged();
    reload();
}

void CourseMapItem::setPlayerX(const double value) {
    if (qFuzzyCompare(m_playerX, value)) return;
    m_playerX = value;
    emit playerChanged();
    emit measurementChanged();
    update();
}

void CourseMapItem::setPlayerY(const double value) {
    if (qFuzzyCompare(m_playerY, value)) return;
    m_playerY = value;
    emit playerChanged();
    emit measurementChanged();
    update();
}

void CourseMapItem::setPlayerVisible(const bool value) {
    if (m_playerVisible == value) return;
    m_playerVisible = value;
    emit playerChanged();
    emit measurementChanged();
    update();
}

void CourseMapItem::setZoom(const double value) {
    const double bounded = std::clamp(value, 0.75, 6.0);
    if (qFuzzyCompare(m_zoom, bounded)) return;
    m_zoom = bounded;
    emit viewTransformChanged();
    update();
}

void CourseMapItem::setPanX(const double value) {
    if (qFuzzyCompare(m_panX, value)) return;
    m_panX = value;
    emit viewTransformChanged();
    update();
}

void CourseMapItem::setPanY(const double value) {
    if (qFuzzyCompare(m_panY, value)) return;
    m_panY = value;
    emit viewTransformChanged();
    update();
}

void CourseMapItem::setRotationDegrees(const double value) {
    double normalized = std::fmod(value, 360.0);
    if (normalized < -180.0) normalized += 360.0;
    if (normalized > 180.0) normalized -= 360.0;
    if (qFuzzyCompare(m_rotationDegrees, normalized)) return;
    m_rotationDegrees = normalized;
    emit viewTransformChanged();
    update();
}

void CourseMapItem::setMeasurementPoints(const QVariantList& points) {
    if (m_measurementPoints == points) return;
    m_measurementPoints = points;
    emit measurementChanged();
    update();
}

void CourseMapItem::setMeasurementFromPlayer(const bool value) {
    if (m_measurementFromPlayer == value) return;
    m_measurementFromPlayer = value;
    emit measurementChanged();
    update();
}

void CourseMapItem::setMeasurementToTarget(const bool value) {
    if (m_measurementToTarget == value) return;
    m_measurementToTarget = value;
    emit measurementChanged();
    update();
}

void CourseMapItem::setMetric(const bool value) {
    if (m_metric == value) return;
    m_metric = value;
    emit measurementChanged();
    update();
}

double CourseMapItem::measuredDistanceMetres() const {
    const auto points = measurementPath();
    double distance = 0.0;
    for (qsizetype index = 1; index < points.size(); ++index) {
        const QPointF delta = points[index] - points[index - 1];
        distance += std::hypot(delta.x(), delta.y());
    }
    return distance;
}

bool CourseMapItem::ready() const { return m_ready; }
QString CourseMapItem::errorText() const { return m_errorText; }

QVariantMap CourseMapItem::mapPointAt(const double itemX,
                                      const double itemY) const {
    bool invertible = false;
    const QTransform inverse = mapToItemTransform().inverted(&invertible);
    if (!invertible) return {};
    const QPointF point = inverse.map(QPointF{itemX, itemY});
    return {
        {QStringLiteral("x"), point.x()},
        {QStringLiteral("y"), point.y()},
        {QStringLiteral("inside"), m_viewBox.contains(point)},
    };
}

void CourseMapItem::reload() {
    const bool previousReady = m_ready;
    const QString previousError = m_errorText;
    m_ready = false;
    m_errorText.clear();
    m_targetVisible = false;
    m_measurementPoints.clear();
    if (m_modelSource.isEmpty()) {
        if (previousReady != m_ready || previousError != m_errorText)
            emit statusChanged();
        emit measurementChanged();
        update();
        return;
    }
    const QString path = m_modelSource.isLocalFile()
                             ? m_modelSource.toLocalFile()
                             : m_modelSource.toString().startsWith(
                                   QStringLiteral("qrc:/"))
                                   ? QStringLiteral(":") +
                                         m_modelSource.path()
                                   : m_modelSource.toString();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        failLoad(tr("Course map is unavailable."), path);
        return;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        failLoad(tr("Course map data is invalid."), path);
        return;
    }
    const QJsonObject root = document.object();
    const QJsonObject model =
        root.contains(QStringLiteral("model"))
            ? root.value(QStringLiteral("model")).toObject()
            : root;
    const QJsonValue holesValue = model.value(QStringLiteral("holes"));
    if (!holesValue.isArray()) {
        failLoad(tr("Course map data is invalid."), path);
        return;
    }
    const QJsonArray holes = holesValue.toArray();
    QJsonObject selected;
    for (const auto& value : holes) {
        const auto candidate = value.toObject();
        if (candidate.value(QStringLiteral("number")).toInt() == m_hole) {
            selected = candidate;
            break;
        }
    }
    if (selected.isEmpty() && !holes.isEmpty()) selected = holes.first().toObject();
    if (selected.isEmpty()) {
        failLoad(tr("This hole has no map geometry."), path);
        return;
    }
    const auto viewBox = selected.value(QStringLiteral("viewBox")).toArray();
    if (viewBox.size() != 4 || viewBox[2].toDouble() <= 0.0 ||
        viewBox[3].toDouble() <= 0.0) {
        failLoad(tr("Course map data is invalid."), path);
        return;
    }
    m_viewBox = {viewBox[0].toDouble(), viewBox[1].toDouble(),
                 viewBox[2].toDouble(), viewBox[3].toDouble()};
    for (const auto& value : selected.value(QStringLiteral("features")).toArray()) {
        const auto feature = value.toObject();
        if (feature.value(QStringLiteral("kind")).toString() !=
            QStringLiteral("pin")) {
            continue;
        }
        const auto anchor = feature.value(QStringLiteral("anchor")).toObject();
        if (anchor.contains(QStringLiteral("x")) &&
            anchor.contains(QStringLiteral("y"))) {
            m_target = {anchor.value(QStringLiteral("x")).toDouble(),
                        anchor.value(QStringLiteral("y")).toDouble()};
            m_targetVisible = true;
        }
        break;
    }
    const QByteArray svg = buildSvg(selected);
    if (!m_renderer.load(svg)) {
        failLoad(tr("Course map could not be rendered."), path);
        return;
    }
    m_ready = true;
    m_errorText.clear();
    qInfo().noquote() << "Course map ready:" << path << "hole" << m_hole;
    emit statusChanged();
    emit measurementChanged();
    update();
}

void CourseMapItem::failLoad(const QString &message, const QString &path) {
    m_renderer.load(QByteArray{});
    m_ready = false;
    m_errorText = message;
    m_targetVisible = false;
    qWarning().noquote() << "Course map load failed:" << path << "hole" << m_hole
                         << '-' << message;
    emit statusChanged();
    emit measurementChanged();
    emit loadError(message);
    update();
}

QByteArray CourseMapItem::buildSvg(const QJsonObject& hole) const {
    QByteArray svg;
    QXmlStreamWriter writer(&svg);
    writer.writeStartDocument();
    writer.writeStartElement(QStringLiteral("svg"));
    writer.writeDefaultNamespace(QStringLiteral("http://www.w3.org/2000/svg"));
    writer.writeAttribute(
        QStringLiteral("viewBox"),
        QStringLiteral("%1 %2 %3 %4")
            .arg(m_viewBox.x())
            .arg(m_viewBox.y())
            .arg(m_viewBox.width())
            .arg(m_viewBox.height()));

    const QString clip = hole.value(QStringLiteral("clip")).toString();
    if (!clip.isEmpty()) {
        writer.writeStartElement(QStringLiteral("defs"));
        writer.writeStartElement(QStringLiteral("clipPath"));
        writer.writeAttribute(QStringLiteral("id"), QStringLiteral("corridor"));
        writer.writeEmptyElement(QStringLiteral("path"));
        writer.writeAttribute(QStringLiteral("d"), clip);
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEmptyElement(QStringLiteral("path"));
        writer.writeAttribute(QStringLiteral("d"), clip);
        writer.writeAttribute(QStringLiteral("fill"), colorFor(QStringLiteral("rough")).name());
    }

    writer.writeStartElement(QStringLiteral("g"));
    if (!clip.isEmpty()) {
        writer.writeAttribute(QStringLiteral("clip-path"),
                              QStringLiteral("url(#corridor)"));
    }
    for (const auto& value : hole.value(QStringLiteral("features")).toArray()) {
        const QJsonObject feature = value.toObject();
        const QString kind = feature.value(QStringLiteral("kind")).toString();
        writer.writeEmptyElement(QStringLiteral("path"));
        writer.writeAttribute(QStringLiteral("d"),
                              feature.value(QStringLiteral("d")).toString());
        const bool line = kind == QStringLiteral("hole_line") ||
                          kind == QStringLiteral("path");
        writer.writeAttribute(QStringLiteral("fill"),
                              line ? QStringLiteral("none")
                                   : colorFor(kind).name());
        if (line) {
            writer.writeAttribute(QStringLiteral("stroke"),
                                  colorFor(kind).name());
            writer.writeAttribute(QStringLiteral("stroke-width"),
                                  kind == QStringLiteral("hole_line")
                                      ? QStringLiteral("1.6")
                                      : QStringLiteral("2.4"));
            if (kind == QStringLiteral("hole_line")) {
                writer.writeAttribute(QStringLiteral("stroke-dasharray"),
                                      QStringLiteral("6 5"));
            }
        }
    }
    writer.writeEndElement();
    writer.writeEndElement();
    writer.writeEndDocument();
    return svg;
}

QColor CourseMapItem::colorFor(const QString& kind) const {
    const QColor color(m_colors.value(kind).toString());
    return color.isValid() ? color : QColor(QStringLiteral("#607064"));
}

QTransform CourseMapItem::mapToItemTransform() const {
    const double safeWidth = std::max(1.0, m_viewBox.width());
    const double safeHeight = std::max(1.0, m_viewBox.height());
    const double radians = m_rotationDegrees * std::acos(-1.0) / 180.0;
    const double cosine = std::abs(std::cos(radians));
    const double sine = std::abs(std::sin(radians));
    const double rotatedWidth = safeWidth * cosine + safeHeight * sine;
    const double rotatedHeight = safeWidth * sine + safeHeight * cosine;
    const double fitScale =
        std::min(width() / rotatedWidth, height() / rotatedHeight);
    QTransform transform;
    transform.translate(width() / 2.0 + m_panX, height() / 2.0 + m_panY);
    transform.rotate(m_rotationDegrees);
    transform.scale(fitScale * m_zoom, fitScale * m_zoom);
    transform.translate(-m_viewBox.center().x(), -m_viewBox.center().y());
    return transform;
}

QVector<QPointF> CourseMapItem::measurementPath() const {
    QVector<QPointF> points;
    if (m_measurementFromPlayer && m_playerVisible) {
        points.push_back({m_playerX, m_playerY});
    }
    for (const auto& value : m_measurementPoints) {
        const auto point = value.toMap();
        bool xValid = false;
        bool yValid = false;
        const double x = point.value(QStringLiteral("x")).toDouble(&xValid);
        const double y = point.value(QStringLiteral("y")).toDouble(&yValid);
        if (xValid && yValid) points.push_back({x, y});
    }
    if (m_measurementToTarget && m_targetVisible) points.push_back(m_target);
    return points;
}

void CourseMapItem::paintMeasurements(QPainter* painter) const {
    const auto points = measurementPath();
    if (points.size() < 2) return;

    const QTransform transform = mapToItemTransform();
    QVector<QPointF> screenPoints;
    screenPoints.reserve(points.size());
    for (const auto& point : points) screenPoints.push_back(transform.map(point));

    painter->save();
    painter->setClipRect(boundingRect());
    QPen linePen(QColor(QStringLiteral("#F7F8F2")), 2.25,
                 Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(linePen);
    for (qsizetype index = 1; index < screenPoints.size(); ++index) {
        painter->drawLine(screenPoints[index - 1], screenPoints[index]);
    }

    QFont labelFont(QStringLiteral("Inter"));
    labelFont.setPixelSize(13);
    labelFont.setWeight(QFont::DemiBold);
    painter->setFont(labelFont);
    const QFontMetrics metrics(labelFont);
    for (qsizetype index = 1; index < points.size(); ++index) {
        const QPointF delta = points[index] - points[index - 1];
        const double metres = std::hypot(delta.x(), delta.y());
        const int displayDistance = static_cast<int>(
            std::round(m_metric ? metres : metres * 1.0936133));
        const QString label = QStringLiteral("%1 %2")
                                  .arg(displayDistance)
                                  .arg(m_metric ? QStringLiteral("m")
                                                : QStringLiteral("yd"));
        const QSize textSize = metrics.size(Qt::TextSingleLine, label);
        QPointF labelCentre = (screenPoints[index - 1] + screenPoints[index]) / 2.0;
        labelCentre += QPointF{0.0, index % 2 == 0 ? -22.0 : 22.0};
        QRectF labelRect(labelCentre.x() - textSize.width() / 2.0 - 7.0,
                         labelCentre.y() - textSize.height() / 2.0 - 4.0,
                         textSize.width() + 14.0, textSize.height() + 8.0);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(QStringLiteral("#D9101211")));
        painter->drawRoundedRect(labelRect, 7.0, 7.0);
        painter->setPen(QColor(QStringLiteral("#F7F8F2")));
        painter->drawText(labelRect, Qt::AlignCenter, label);
    }

    painter->setPen(QPen(QColor(QStringLiteral("#101211")), 2.0));
    painter->setBrush(QColor(QStringLiteral("#F7F8F2")));
    for (qsizetype index = 1; index < screenPoints.size(); ++index) {
        if (m_measurementToTarget && m_targetVisible &&
            index == screenPoints.size() - 1) {
            continue;
        }
        painter->drawEllipse(screenPoints[index], 6.5, 6.5);
    }
    painter->restore();
}

void CourseMapItem::paintPlayer(QPainter* painter) const {
    if (!m_playerVisible || m_viewBox.width() <= 0.0 ||
        m_viewBox.height() <= 0.0) {
        return;
    }
    const QPointF player =
        mapToItemTransform().map(QPointF{m_playerX, m_playerY});
    painter->save();
    painter->setClipRect(boundingRect());
    painter->setPen(QPen(QColor(QStringLiteral("#101211")), 2.0));
    painter->setBrush(QColor(QStringLiteral("#F7F8F2")));
    painter->drawEllipse(player, 6.0, 6.0);
    painter->restore();
}

} // namespace opencaddie::ui
