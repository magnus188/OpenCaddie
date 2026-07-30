#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>

#include <optional>

namespace opencaddie::courses {

struct ZipContents {
    QMap<QString, QByteArray> files;
    qint64 totalUncompressedBytes = 0;
};

class StoredZipReader {
public:
    static std::optional<ZipContents> read(const QByteArray& archive,
                                           QString* error);
    static bool isSafeRelativePath(const QString& path);
};

} // namespace opencaddie::courses

