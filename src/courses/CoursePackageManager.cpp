#include "courses/CoursePackageManager.h"

#include "courses/StoredZipReader.h"

#include <QCryptographicHash>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QUuid>

namespace opencaddie::courses {
namespace {
std::optional<storage::CachedCourse> fail(QString* error,
                                          const QString& message) {
    if (error) *error = message;
    return std::nullopt;
}

bool validRenderModel(const QByteArray &bytes) {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
        return false;
    const QJsonObject root = document.object();
    const QJsonObject model = root.contains(QStringLiteral("model"))
                                  ? root.value(QStringLiteral("model")).toObject()
                                  : root;
    const QJsonArray holes = model.value(QStringLiteral("holes")).toArray();
    if (holes.isEmpty()) return false;
    for (const auto &value : holes) {
        const QJsonObject hole = value.toObject();
        const QJsonArray viewBox = hole.value(QStringLiteral("viewBox")).toArray();
        if (hole.value(QStringLiteral("number")).toInt() <= 0 ||
            viewBox.size() != 4 || viewBox.at(2).toDouble() <= 0.0 ||
            viewBox.at(3).toDouble() <= 0.0 ||
            !hole.value(QStringLiteral("features")).isArray()) {
            return false;
        }
    }
    return true;
}
} // namespace

CoursePackageManager::CoursePackageManager(
    QString coursesRoot, storage::CourseRepository repository)
    : m_coursesRoot(std::move(coursesRoot)),
      m_repository(std::move(repository)) {}

std::optional<storage::CachedCourse>
CoursePackageManager::install(const QByteArray& archive, QString* error) {
    const auto contents = StoredZipReader::read(archive, error);
    if (!contents) return std::nullopt;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(
        contents->files.value(QStringLiteral("manifest.json")), &parseError);
    if (!document.isObject() || parseError.error != QJsonParseError::NoError) {
        return fail(error, QStringLiteral("Course manifest is invalid JSON."));
    }
    const QJsonObject manifest = document.object();
    if (manifest.value(QStringLiteral("schemaVersion")).toString() !=
        QStringLiteral("2")) {
        return fail(error, QStringLiteral("Course package schema is not supported."));
    }
    const QJsonObject course = manifest.value(QStringLiteral("course")).toObject();
    const QJsonObject licensing =
        manifest.value(QStringLiteral("licensing")).toObject();
    const QString slug = course.value(QStringLiteral("slug")).toString();
    const QString name = course.value(QStringLiteral("name")).toString();
    const QString version = course.value(QStringLiteral("version")).toString();
    const QString attribution =
        licensing.value(QStringLiteral("attribution")).toString();
    if (safeSegment(slug) != slug || name.isEmpty() || version.isEmpty() ||
        attribution.isEmpty() ||
        licensing.value(QStringLiteral("dataLicense")).toString() !=
            QStringLiteral("ODbL-1.0")) {
        return fail(error,
                    QStringLiteral("Course manifest lacks identity or ODbL attribution."));
    }

    for (const auto& assetValue :
         manifest.value(QStringLiteral("assets")).toArray()) {
        const QJsonObject asset = assetValue.toObject();
        const QString filename = asset.value(QStringLiteral("file")).toString();
        if (!contents->files.contains(filename)) {
            return fail(error, QStringLiteral("Course asset is missing: %1")
                                   .arg(filename));
        }
        const QByteArray bytes = contents->files.value(filename);
        const auto byteSize =
            asset.value(QStringLiteral("byteSize")).toVariant().toLongLong();
        const QString expectedHash =
            asset.value(QStringLiteral("sha256")).toString().toLower();
        const QString actualHash = QString::fromLatin1(
            QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
        if (byteSize != bytes.size() || expectedHash != actualHash) {
            return fail(error, QStringLiteral("Course asset verification failed: %1")
                                   .arg(filename));
        }
    }
    if (!contents->files.contains(QStringLiteral("render-model.json")) ||
        !contents->files.contains(QStringLiteral("navigation.json"))) {
        return fail(error, QStringLiteral("Course package is incomplete."));
    }
    if (!validRenderModel(
            contents->files.value(QStringLiteral("render-model.json")))) {
        return fail(error, QStringLiteral("Course render model is invalid."));
    }

    QDir root(m_coursesRoot);
    if (!root.mkpath(QStringLiteral("."))) {
        return fail(error, QStringLiteral("Course storage is unavailable."));
    }
    const QString stagingName =
        QStringLiteral(".install-%1").arg(QUuid::createUuid().toString(
            QUuid::WithoutBraces));
    const QString stagingPath = root.filePath(stagingName);
    if (!QDir().mkpath(stagingPath)) {
        return fail(error, QStringLiteral("Could not create staging directory."));
    }
    for (auto iterator = contents->files.cbegin();
         iterator != contents->files.cend(); ++iterator) {
        if (!writeFile(stagingPath, iterator.key(), iterator.value(), error)) {
            QDir(stagingPath).removeRecursively();
            return std::nullopt;
        }
    }

    const QString slugRoot = root.filePath(slug);
    if (!QDir().mkpath(slugRoot)) {
        QDir(stagingPath).removeRecursively();
        return fail(error, QStringLiteral("Could not create course directory."));
    }
    const QString finalPath = QDir(slugRoot).filePath(safeSegment(version));
    if (QDir(finalPath).exists()) {
        QDir(stagingPath).removeRecursively();
    } else if (!QDir().rename(stagingPath, finalPath)) {
        QDir(stagingPath).removeRecursively();
        return fail(error, QStringLiteral("Could not activate the course package."));
    }

    const auto quality =
        manifest.value(QStringLiteral("quality")).toObject();
    const storage::CachedCourse installed{
        slug,
        version,
        name,
        finalPath,
        quality.value(QStringLiteral("score")).toInt(),
        contents->totalUncompressedBytes,
        attribution,
    };
    if (!m_repository.install(installed)) {
        return fail(error, QStringLiteral("Could not register installed course."));
    }
    return installed;
}

bool CoursePackageManager::remove(const QString& slug, const QString& version,
                                  QString* error) {
    if (safeSegment(slug) != slug || version.isEmpty()) {
        if (error) *error = QStringLiteral("Invalid course identity.");
        return false;
    }
    const QString path =
        QDir(QDir(m_coursesRoot).filePath(slug)).filePath(safeSegment(version));
    const QString absoluteRoot = QDir(m_coursesRoot).absolutePath() + '/';
    const QString absolutePath = QDir(path).absolutePath();
    if (!absolutePath.startsWith(absoluteRoot) ||
        (QDir(path).exists() && !QDir(path).removeRecursively())) {
        if (error) *error = QStringLiteral("Could not remove course files.");
        return false;
    }
    return m_repository.remove(slug, version);
}

QString CoursePackageManager::coursesRoot() const { return m_coursesRoot; }

QString CoursePackageManager::safeSegment(const QString& value) {
    QString safe = value;
    safe.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")),
                 QStringLiteral("_"));
    return safe;
}

bool CoursePackageManager::writeFile(const QString& root,
                                     const QString& relativePath,
                                     const QByteArray& bytes, QString* error) {
    if (!StoredZipReader::isSafeRelativePath(relativePath)) {
        if (error) *error = QStringLiteral("Unsafe course asset path.");
        return false;
    }
    const QString destination = QDir(root).filePath(relativePath);
    if (!QDir().mkpath(QFileInfo(destination).absolutePath())) {
        if (error) *error = QStringLiteral("Could not create asset directory.");
        return false;
    }
    QSaveFile file(destination);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() ||
        !file.commit()) {
        if (error) *error = QStringLiteral("Could not write course asset.");
        return false;
    }
    return true;
}

} // namespace opencaddie::courses
