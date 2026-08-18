#include "ui/CourseMapItem.h"

#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>
#include <QUrl>

#include <cstdlib>
#include <iostream>

namespace {
void require(const bool condition, const char *message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void writeFile(const QString &path, const QByteArray &contents) {
    QFile file(path);
    require(file.open(QIODevice::WriteOnly), "fixture opens for writing");
    require(file.write(contents) == contents.size(), "fixture writes completely");
}

QImage render(opencaddie::ui::CourseMapItem &map) {
    QImage image(static_cast<int>(map.width()), static_cast<int>(map.height()),
                 QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    map.paint(&painter);
    painter.end();
    return image;
}

int changedPixels(const QImage &first, const QImage &second) {
    require(first.size() == second.size(), "render comparison has matching size");
    int changed = 0;
    for (int y = 0; y < first.height(); ++y) {
        const auto *firstLine =
            reinterpret_cast<const QRgb *>(first.constScanLine(y));
        const auto *secondLine =
            reinterpret_cast<const QRgb *>(second.constScanLine(y));
        for (int x = 0; x < first.width(); ++x) {
            if (firstLine[x] != secondLine[x]) ++changed;
        }
    }
    return changed;
}
} // namespace

int main(int argc, char **argv) {
    QGuiApplication application(argc, argv);
    QTemporaryDir temporary;
    require(temporary.isValid(), "temporary directory");

    const QByteArray model = R"JSON({
      "schemaVersion":"1",
      "model":{"holes":[
        {"number":1,"viewBox":[0,0,120,220],"clip":"M5 5H115V215H5Z","features":[
          {"kind":"wood","d":"M5 5H115V215H5Z"},
          {"kind":"fairway","d":"M35 20H85V190H35Z"},
          {"kind":"bunker","d":"M18 130H40V154H18Z"},
          {"kind":"green","d":"M34 162H88V212H34Z"},
          {"kind":"tee","d":"M48 12H72V28H48Z"},
          {"kind":"path","d":"M104 10V210"},
          {"kind":"hole_line","d":"M60 20V190"},
          {"kind":"pin","d":"M57 180H63V198H57Z","anchor":{"x":60,"y":190}}
        ]},
        {"number":2,"viewBox":[0,0,100,100],"features":[
          {"kind":"fairway","d":"M10 10H90V90H10Z"},
          {"kind":"green","d":"M30 30H70V70H30Z"},
          {"kind":"pin","d":"M48 44H52V60H48Z","anchor":{"x":50,"y":50}}
        ]}
      ]}
    })JSON";
    const QString validPath = temporary.filePath(QStringLiteral("render-model.json"));
    writeFile(validPath, model);

    opencaddie::ui::CourseMapItem map;
    map.setWidth(320);
    map.setHeight(420);
    map.setHole(1);
    map.setModelSource(QUrl::fromLocalFile(validPath));
    require(map.ready(), "wrapped OpenGolfMap model is ready");
    require(map.errorText().isEmpty(), "valid model has no error");

    QImage image = render(map);
    int renderedPixels = 0;
    for (int y = 0; y < image.height(); ++y) {
        const auto *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(line[x]) > 0) ++renderedPixels;
        }
    }
    require(renderedPixels > 4'000, "course geometry renders non-empty pixels");

    const QVariantList shotTrail{
        QVariantMap{{QStringLiteral("sequence"), 1},
                    {QStringLiteral("type"), QStringLiteral("drive")},
                    {QStringLiteral("startX"), 60.0},
                    {QStringLiteral("startY"), 20.0},
                    {QStringLiteral("endX"), 60.0},
                    {QStringLiteral("endY"), 150.0},
                    {QStringLiteral("distance"), 130.0}},
        QVariantMap{{QStringLiteral("sequence"), 2},
                    {QStringLiteral("type"), QStringLiteral("chip")},
                    {QStringLiteral("endX"), 70.0},
                    {QStringLiteral("endY"), 170.0}},
        QVariantMap{{QStringLiteral("sequence"), 3},
                    {QStringLiteral("type"), QStringLiteral("unknown")}},
    };
    map.setShotTrail(shotTrail);
    require(map.shotTrail().size() == 3,
            "locationless strokes remain in the trail payload");
    const QImage labelledTrail = render(map);
    require(changedPixels(image, labelledTrail) > 150,
            "segments, numbered landings and endpoint-only markers render");

    map.setShowShotTrailLabels(false);
    const QImage compactTrail = render(map);
    require(changedPixels(labelledTrail, compactTrail) > 20,
            "compact trail suppresses distance labels");

    map.setZoom(1.8);
    map.setPanX(18.0);
    map.setPanY(-12.0);
    map.setRotationDegrees(24.0);
    const QImage transformedTrail = render(map);
    require(changedPixels(compactTrail, transformedTrail) > 500,
            "trail follows zoom, pan and rotation transforms");

    map.setZoom(1.0);
    map.setPanX(0.0);
    map.setPanY(0.0);
    map.setRotationDegrees(0.0);

    map.setHole(2);
    require(map.ready(), "second hole selection remains renderable");
    map.setPlayerVisible(false);
    require(map.ready(), "rendering does not require GPS");

    const QString invalidPath = temporary.filePath(QStringLiteral("invalid.json"));
    writeFile(invalidPath, QByteArrayLiteral("{not-json"));
    map.setModelSource(QUrl::fromLocalFile(invalidPath));
    require(!map.ready(), "invalid model clears ready state");
    require(!map.errorText().isEmpty(), "invalid model exposes a concise error");

    std::cout << "Course map tests passed\n";
    return EXIT_SUCCESS;
}
