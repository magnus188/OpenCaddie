#include "ui/CourseMapItem.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>
#include <QXmlStreamWriter>

#include <algorithm>

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
    const QRectF target = renderedMapRect();
    if (m_renderer.isValid()) {
        m_renderer.render(painter, target);
    }
    if (!m_playerVisible || m_viewBox.width() <= 0.0 ||
        m_viewBox.height() <= 0.0) {
        return;
    }
    const double scale = std::min(target.width() / m_viewBox.width(),
                                  target.height() / m_viewBox.height());
    const QPointF player{
        target.left() + (m_playerX - m_viewBox.left()) * scale,
        target.top() + (m_playerY - m_viewBox.top()) * scale,
    };
    painter->setPen(QPen(QColor(QStringLiteral("#101211")), 2.0));
    painter->setBrush(QColor(QStringLiteral("#F7F8F2")));
    painter->drawEllipse(player, 6.0, 6.0);
}

QUrl CourseMapItem::modelSource() const { return m_modelSource; }
int CourseMapItem::hole() const { return m_hole; }
QVariantMap CourseMapItem::colors() const { return m_colors; }
double CourseMapItem::playerX() const { return m_playerX; }
double CourseMapItem::playerY() const { return m_playerY; }
bool CourseMapItem::playerVisible() const { return m_playerVisible; }

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
    update();
}

void CourseMapItem::setPlayerY(const double value) {
    if (qFuzzyCompare(m_playerY, value)) return;
    m_playerY = value;
    emit playerChanged();
    update();
}

void CourseMapItem::setPlayerVisible(const bool value) {
    if (m_playerVisible == value) return;
    m_playerVisible = value;
    emit playerChanged();
    update();
}

void CourseMapItem::reload() {
    if (m_modelSource.isEmpty()) return;
    const QString path = m_modelSource.isLocalFile()
                             ? m_modelSource.toLocalFile()
                             : m_modelSource.toString().startsWith(
                                   QStringLiteral("qrc:/"))
                                   ? QStringLiteral(":") +
                                         m_modelSource.path()
                                   : m_modelSource.toString();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        emit loadError(tr("Could not open the course render model."));
        return;
    }
    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    const QJsonObject model =
        root.contains(QStringLiteral("model"))
            ? root.value(QStringLiteral("model")).toObject()
            : root;
    const QJsonArray holes = model.value(QStringLiteral("holes")).toArray();
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
        emit loadError(tr("The course has no renderable holes."));
        return;
    }
    const auto viewBox = selected.value(QStringLiteral("viewBox")).toArray();
    if (viewBox.size() == 4) {
        m_viewBox = {viewBox[0].toDouble(), viewBox[1].toDouble(),
                     viewBox[2].toDouble(), viewBox[3].toDouble()};
    }
    const QByteArray svg = buildSvg(selected);
    if (!m_renderer.load(svg)) {
        emit loadError(tr("Could not render the course geometry."));
    }
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

QRectF CourseMapItem::renderedMapRect() const {
    if (m_viewBox.width() <= 0.0 || m_viewBox.height() <= 0.0) {
        return boundingRect();
    }
    const double scale = std::min(width() / m_viewBox.width(),
                                  height() / m_viewBox.height());
    const QSizeF size{m_viewBox.width() * scale, m_viewBox.height() * scale};
    return {(width() - size.width()) / 2.0,
            (height() - size.height()) / 2.0, size.width(), size.height()};
}

} // namespace opencaddie::ui

