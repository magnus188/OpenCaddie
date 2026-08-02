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

    QImage image(320, 420, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    map.paint(&painter);
    painter.end();
    int renderedPixels = 0;
    for (int y = 0; y < image.height(); ++y) {
        const auto *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(line[x]) > 0) ++renderedPixels;
        }
    }
    require(renderedPixels > 4'000, "course geometry renders non-empty pixels");

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
