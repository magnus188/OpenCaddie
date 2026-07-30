#include "courses/CoursePackageManager.h"
#include "courses/StoredZipReader.h"
#include "storage/Database.h"

#include <QCryptographicHash>
#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtEndian>

#include <cstdlib>
#include <iostream>

namespace {
template <typename T>
void appendLittleEndian(QByteArray& output, const T value) {
    const T littleEndian = qToLittleEndian(value);
    output.append(reinterpret_cast<const char*>(&littleEndian), sizeof(T));
}

void appendStoredEntry(QByteArray& archive, const QString& name,
                       const QByteArray& contents) {
    const QByteArray encodedName = name.toUtf8();
    appendLittleEndian<quint32>(archive, 0x04034b50);
    appendLittleEndian<quint16>(archive, 20);
    appendLittleEndian<quint16>(archive, 0);
    appendLittleEndian<quint16>(archive, 0);
    appendLittleEndian<quint16>(archive, 0);
    appendLittleEndian<quint16>(archive, 0);
    appendLittleEndian<quint32>(archive, 0);
    appendLittleEndian<quint32>(archive, contents.size());
    appendLittleEndian<quint32>(archive, contents.size());
    appendLittleEndian<quint16>(archive, encodedName.size());
    appendLittleEndian<quint16>(archive, 0);
    archive.append(encodedName);
    archive.append(contents);
}

QJsonObject asset(const QString& file, const QByteArray& bytes,
                  const bool corruptHash = false) {
    const QByteArray hash =
        QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex();
    return {
        {QStringLiteral("file"), file},
        {QStringLiteral("byteSize"), bytes.size()},
        {QStringLiteral("sha256"),
         corruptHash ? QString(64, QLatin1Char('0'))
                     : QString::fromLatin1(hash)},
    };
}

QByteArray package(const bool corruptHash = false,
                   const bool includeAttribution = true,
                   const QString& schema = QStringLiteral("2")) {
    const QByteArray render = R"({"schemaVersion":"1","holes":[]})";
    const QByteArray navigation = R"({"schemaVersion":"1","holes":[]})";
    const QJsonObject manifest{
        {QStringLiteral("schemaVersion"), schema},
        {QStringLiteral("course"),
         QJsonObject{
             {QStringLiteral("slug"), QStringLiteral("test-course")},
             {QStringLiteral("name"), QStringLiteral("Test Course")},
             {QStringLiteral("version"), QStringLiteral("2026-07-30")},
         }},
        {QStringLiteral("licensing"),
         QJsonObject{
             {QStringLiteral("dataLicense"), QStringLiteral("ODbL-1.0")},
             {QStringLiteral("attribution"),
              includeAttribution
                  ? QStringLiteral("© OpenStreetMap contributors")
                  : QString{}},
         }},
        {QStringLiteral("quality"),
         QJsonObject{{QStringLiteral("score"), 92}}},
        {QStringLiteral("assets"),
         QJsonArray{
             asset(QStringLiteral("render-model.json"), render, corruptHash),
             asset(QStringLiteral("navigation.json"), navigation),
         }},
    };

    QByteArray archive;
    appendStoredEntry(
        archive, QStringLiteral("manifest.json"),
        QJsonDocument(manifest).toJson(QJsonDocument::Compact));
    appendStoredEntry(archive, QStringLiteral("render-model.json"), render);
    appendStoredEntry(archive, QStringLiteral("navigation.json"), navigation);
    return archive;
}

void require(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
} // namespace

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);
    QTemporaryDir temporary;
    require(temporary.isValid(), "temporary directory");

    opencaddie::storage::Database database;
    require(database.open(QDir(temporary.path()).filePath("user.sqlite")),
            "database opens");
    opencaddie::storage::CourseRepository repository(database.connection());
    opencaddie::courses::CoursePackageManager manager(
        QDir(temporary.path()).filePath("courses"), repository);

    QString error;
    require(!manager.install(package(true), &error),
            "corrupt asset hash is rejected");
    require(repository.list().isEmpty(),
            "rejected package does not change installed courses");

    require(!manager.install(package(false, false), &error),
            "missing attribution is rejected");
    require(!manager.install(package(false, true, QStringLiteral("99")), &error),
            "incompatible schema is rejected");

    const auto installed = manager.install(package(), &error);
    require(installed.has_value(), "valid package installs");
    require(QFileInfo::exists(
                QDir(installed->path).filePath("render-model.json")),
            "semantic render model is activated");
    require(repository.list().size() == 1, "course is registered once");

    require(manager.remove(installed->slug, installed->version, &error),
            "installed package can be removed");
    require(!QDir(installed->path).exists(), "course files are removed");
    require(repository.list().isEmpty(), "course registration is removed");

    QByteArray unsafe;
    appendStoredEntry(unsafe, QStringLiteral("../manifest.json"), QByteArray{});
    require(!opencaddie::courses::StoredZipReader::read(unsafe, &error),
            "path traversal entry is rejected");

    std::cout << "Course package tests passed\n";
    return EXIT_SUCCESS;
}
